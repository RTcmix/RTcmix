/* JCHOR - random-wait chorus instrument based on Paul Lansky's chor
           and Doug Scott's trans code

   p0  = output start time
   p1  = input start time
   p2  = output duration
   p3  = input duration (not input end time)
   p4  = maintain input duration, regardless of transposition (1: yes, 0: no)
   p5  = transposition (8ve.pc)
   p6  = number of voices (minimum of 1)
   p7  = minimum grain amplitude
   p8  = maximum grain amplitude
   p9  = minimum grain wait (seconds)
   p10 = maximum grain wait (seconds)
   p11 = seed (0 - 1)
   p12 = input channel [optional]
         (If p12 is missing and input has > 1 chan, input channels averaged.)

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve. By default, the amplitude envelope is updated 1000
   times per second, but this can be changed by calling reset() with a
   different value.

   Assumes function table 2 is grain window function. (Try gen 25.)

   Output can be either mono or stereo. If it's stereo, the program randomly
   distributes the voices across the stereo field.

   Notes on p4 (maintain input duration)...

      Because the transposition method doesn't try to maintain duration -- it
      works like the speed control on a tape deck -- you have an option about
      the way to handle the duration of the input read:

      - If p4 is 1, the grain length after transposition will be the same as
        that given by p3 (input duration). This means that the amount actually
        read from the input file will be shorter or longer than p3, depending
        on the transposition.

      - If p4 is 0, the segment of sound specified by p3 will be read, and the
        grain length will be shorter or longer than p3, depending on the
        transposition.

   Differences between JCHOR and chor (besides RT ability):
      - No limit on input duration or number of voices
      - Transpose the input signal
      - Specify the input channel to use (or an average of them)
      - Specify overall amplitude curve and grain window function via makegens

   John Gibson (jgg9c@virginia.edu), 9/20/98, RT'd 6/24/99.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "JCHOR.h"
#include <rt.h>
#include <rtdefs.h>

extern "C" {
   extern double m_DUR(float [], int);     /* in sys/minc_info.c */
}

//#define DEBUG

#define ENVELOPE_TABLE_SLOT  1
#define WINDOW_FUNC_SLOT     2
#define AVERAGE_CHANS        -1           /* average input chans flag value */
#define WINDOW_CONTROL_RATE  22051.       /* don't skimp on this */

/* local functions */
static double interp(double, double, double, double);



JCHOR::JCHOR() : Instrument()
{
   in = NULL;
   voices = NULL;
   grain = NULL;
   grain_done = 0;
}


JCHOR::~JCHOR()
{
   delete [] in;
   delete [] voices;
   delete [] grain;
}


int JCHOR::init(float p[], int n_args)
{
   float outskip, outdur, maxamp, maxwait;

   outskip = p[0];
   inskip = p[1];
   outdur = p[2];
   indur = p[3];
   maintain_indur = (int)p[4];
   transpose = p[5];
   nvoices = (int)p[6];
   minamp = p[7];
   maxamp = p[8];
   minwait = p[9];
   maxwait = p[10];
   seed = p[11];
   inchan = (n_args > 12) ? (int)p[12] : AVERAGE_CHANS;

   if (n_args < 12)
      die("JCHOR", "Not enough pfields.");

   rtsetinput(inskip, this);
   nsamps = rtsetoutput(outskip, outdur, this);

   if (outputchans > 2)
      die("JCHOR", "Output must have no more than two channels.");

   if (nvoices < 1)
      die("JCHOR", "Must have at least one voice.");

   if (minamp < 0.0 || maxamp <= 0.0 || minamp > maxamp)
      die("JCHOR", "Grain amplitude range confused.");
   ampdiff = maxamp - minamp;

   if (minwait < 0.0 || maxwait < 0.0 || minwait > maxwait)
      die("JCHOR", "Grain wait range confused.");
   waitdiff = (maxwait - minwait) * SR;
   minwait *= SR;

   if (seed < 0.0 || seed > 1.0)
      die("JCHOR", "Seed must be between 0 and 1 inclusive.");

   amparray = floc(ENVELOPE_TABLE_SLOT);
   if (amparray) {
      int len = fsize(ENVELOPE_TABLE_SLOT);
      tableset(outdur, len, amptabs);
   }
   else
      advise("JCHOR", "Setting phrase curve to all 1's.");

   /* MUST do this here, rather than in grain_input_and_transpose,
      because by the time that is called, the makegen for this slot
      may have changed.
   */
   winarray = floc(WINDOW_FUNC_SLOT);
   if (winarray == NULL)
      die("JCHOR", "You haven't made the grain window function (table %d).",
                                                            WINDOW_FUNC_SLOT);
   winarraylen = fsize(WINDOW_FUNC_SLOT);

   skip = (int)(SR / (float)resetval);

   return nsamps;
}


int JCHOR::run()
{
   int   i, j, branch;
   float amp;
   float out[2];
   Voice *v;

   Instrument::run();

   if (!grain_done) {
      in = new float [RTBUFSAMPS * inputchans];
      grain_input_and_transpose();
      setup_voices();
   }

   amp = 1.0;                   /* in case amparray == NULL */

   branch = 0;
   for (i = 0; i < chunksamps; i++) {
      if (--branch < 0) {
         if (amparray)
            amp = tablei(cursamp, amparray, amptabs);
         branch = skip;
      }

      out[0] = out[1] = 0.0;
      for (j = 0, v = voices; j < nvoices; j++, v++) {
         if (v->index++ < 0)
            continue;
         if (v->index >= grainsamps) {
            seed = crandom(seed);
            v->index = (int)(-((seed * waitdiff) + minwait));
            if (outputchans > 1) {
               seed = crandom(seed);
               v->left_amp = seed;
               v->right_amp = 1.0 - v->left_amp;
            }
            seed = crandom(seed);
            v->overall_amp = (seed * ampdiff) + minamp;
         }
         else {
            float sig = grain[v->index] * v->overall_amp;
            if (outputchans > 1) {
               out[0] += sig * v->left_amp;
               out[1] += sig * v->right_amp;
            }
            else
               out[0] += sig;
         }
      }
      out[0] *= amp;
      out[1] *= amp;

      rtaddout(out);
      cursamp++;
   }

   return i;
}


/* --------------------------------------------------------------- interp --- */
/* Cubic spline interpolation.
   Nabbed from Doug Scott's trans.
*/
static double
interp(double y0, double y1, double y2, double t)
{
   register double hy2, hy0, a, b, c;

   a = y0;
   hy0 = y0 / 2.0;
   hy2 =  y2 / 2.0;
   b = (-3.0 * hy0) + (2.0 * y1) - hy2;
   c = hy0 - y1 + hy2;

   return (a + b*t + c*t*t);
}


/* --------------------------------------------------------- setup_voices --- */
int JCHOR::setup_voices()
{
   int   i;
   Voice *v;

   voices = new Voice[nvoices];

   for (i = 0, v = voices; i < nvoices; i++, v++) {
      seed = crandom(seed);
      v->index = (int)(-seed * (grainsamps - 1));
      if (outputchans > 1) {
         seed = crandom(seed);
         v->left_amp = seed;
         v->right_amp = 1.0 - v->left_amp;
      }
      seed = crandom(seed);
      v->overall_amp = (seed * ampdiff) + minamp;
   }
#ifdef DEBUG
   printf("\n%d grainsamps\n", grainsamps);
   printf("Voices:\n");
   for (i = 0, v = voices; i < nvoices; i++, v++)
      printf("%6d: index=%d, left=%g, right=%g, amp=%g\n",
             i, v->index, v->left_amp, v->right_amp, v->overall_amp);
#endif
   return 0;
}


/* -------------------------------------------- grain_input_and_transpose --- */
/* Reads part of a soundfile into a newly allocated array <grain>, transposing
   the signal as it's read, and shaping its amplitude with a window function.

   The method of transposition (adapted from the trans instrument) doesn't
   try to preserve duration. So we have a potential discrepancy between the
   input duration <indur> and the duration of that segment of sound after
   transposition. (For example, if transposition is down an octave, the
   length of the input segment after transposition will be <indur> * 2.)
   There are two ways to handle the discrepancy. Since both ways are useful,
   the caller can choose between them:
      - If <maintain_indur> is true, the grain length after transposition
        will be the same as that given by <indur>. This means that the amount
        actually read from the input file will be shorter or longer than
        <indur>, depending on the transposition.
      - If <maintain_indur> is false, the segment of sound specified by
        <indur> will be read, and the grain length will be shorter or longer
        than <indur>, depending on the transposition.

   <inchan> gives the input channel number to read, or if it's -1, specifies
   that all input channels will be averaged when read into the 1-channel array
   returned by the function. If the input file is mono, <inchan> is ignored.

   <WINDOW_FUNC_SLOT> is the makegen slot of a function used as an amplitude
   curve for the grain. Use gen 25 for a Hanning or Hamming window, or try
   gens 5 or 7 to make other envelope shapes.
*/
int JCHOR::grain_input_and_transpose()
{
   int     i, j, k, n, reset_count, inframes, bufframes;
   int     getflag, incount;
   float   read_indur, store_indur, total_indur, interval, amp = 0.0;
   double  increment, newsig, oldsig, oldersig, frac, counter;

   if (inputchans == 1)
      inchan = 0;

   interval = octpch(transpose);
   increment = cpsoct(10.0 + interval) / cpsoct(10.0);
   if (maintain_indur) {
      read_indur = (double)indur / increment;
      store_indur = indur;
   }
   else {
      read_indur = indur;
      store_indur = (double)indur / increment;
   }
#ifdef DEBUG
   printf("increment=%g, read_indur=%g, store_indur=%g\n",
           increment, read_indur, store_indur);
#endif
#ifdef NOT_YET
   total_indur = (float)m_DUR(NULL, 0);
   if (inskip < 0.0 || read_indur <= 0.0 || inskip + read_indur > total_indur)
      die("JCHOR", "Input file segment out of range.");
#endif

   grainsamps = (int)(store_indur * SR + 0.5);

   grain = new float[grainsamps];

   tableset(store_indur, winarraylen, wintabs);

   inframes = (int)(SR / read_indur);
   bufframes = RTBUFSAMPS;

   reset_count = (int)(SR / WINDOW_CONTROL_RATE);

   getflag = 1;
   incount = 0;            /* frames */
   counter = 0.0;
   oldersig = oldsig = newsig = 0.0;

   k = bufframes;
   for (i = j = 0; i < grainsamps; i++) {
      if (--j < 0) {
         amp = tablei(i, winarray, wintabs);
         j = reset_count;
      }
      while (getflag) {
         int index;
         if (k == bufframes) {              /* time for an input buffer */
            rtgetin(in, this, inputchans * bufframes);
            k = 0;
         }
         index = k * inputchans;
         oldersig = oldsig;
         oldsig = newsig;
         if (inchan == AVERAGE_CHANS) {
            newsig = 0.0;
            for (n = 0; n < inputchans; n++)
               newsig += (double)in[index + n];
            newsig /= (double)inputchans;
         }
         else
            newsig = (double)in[index + inchan];
         incount++;
         k++;
         if (counter - (float)incount < 0.5)
            getflag = 0;
      }
      frac = counter - (double)incount + 2.0;
      grain[i] = (float)interp(oldersig, oldsig, newsig, frac) * amp;
      counter += increment;
      if (counter - (float)incount >= -0.5)
         getflag = 1;
   }

   grain_done = 1;

   return 0;
}


Instrument *makeJCHOR()
{
   JCHOR *inst;

   inst = new JCHOR();
   inst->set_bus_config("JCHOR");

   return inst;
}


void rtprofile()
{
   RT_INTRO("JCHOR", makeJCHOR);
}

