/* TRANSBENDBEND - perform a time-varying transpose on a mono input signal 
   using cubic spline interpolation

   p0 = output start time
   p1 = input start time
   p2 = output duration (time to end if negative)
   p3 = amplitude multiplier
   p4 = gen index for pitch curve
   p5 = input channel [optional, default is 0]
   p6 = percent to left [optional, default is .5]

   Processes only one channel at a time.

   Assumes function table 1 is amplitude curve for the note.
   You can call setline for this.

   Assumes function table 2 is interval curve for the note.
   You can call makegen(2, ...) for this.

   Created from my and John Gibson's TRANSBEND by Doug Scott
	<netdscott@netscape.net> 9/3/2000.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ugens.h>
#include <mixerr.h>
#include "TRANSBEND.h"
#include <rt.h>

//#define DEBUG
//#define DEBUG_FULL


inline float interp(float y0, float y1, float y2, float t)
{
   float hy2, hy0, a, b, c;

   a = y0;
   hy0 = y0 / 2.0f;
   hy2 = y2 / 2.0f;
   b = (-3.0f * hy0) + (2.0f * y1) - hy2;
   c = hy0 - y1 + hy2;

   return (a + b * t + c * t * t);
}

TRANSBEND :: TRANSBEND() : Instrument()
{
   in = NULL;

   incount = 1;
   counter = 0.0;
   get_frame = 1;

   /* clear sample history */
   oldersig = 0.0;
   oldsig = 0.0;
   newsig = 0.0;
}


TRANSBEND :: ~TRANSBEND()
{
   delete [] in;
}


int TRANSBEND :: init(float p[], short n_args)
{
   float outskip, inskip, dur, transp, interval = 0, total_indur, dur_to_read;
   float averageInc;
   int pgen;

   if (n_args < 5)
      die("TRANSBEND", "Wrong number of args.");

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   pgen = (int) p[4];
   inchan = (n_args > 5) ? (int) p[5] : 0;
   pctleft = (n_args > 6) ? p[6] : 0.5;

   if (dur < 0.0)
      dur = -dur - inskip;

   nsamps = rtsetoutput(outskip, dur, this);
   rtsetinput(inskip, this);

   if (inchan >= inputchans)
      die("TRANSBEND", "You asked for channel %d of a %d-channel file.",
                                                       inchan, inputchans);
   pitchtable = floc(pgen);
   if (pitchtable) {
      int plen = fsize(pgen);
      float isum = 0;
      for (int loc = 0; loc < plen; loc++) {
          float pch = pitchtable[loc];
	      isum += octpch(pch);
      }
      interval = isum / plen;
#ifdef DEBUG
      printf("average interval: %g\n", interval);
#endif
      tableset(dur, plen, ptabs);
   }
   else
      die("TRANSBEND", "Unable to load pitch curve!");

   averageInc = (double) cpsoct(10.0 + interval) / cpsoct(10.0);

#ifdef NOTYET
   total_indur = (float) m_DUR(NULL, 0);
   dur_to_read = dur * averageInc;
   if (inskip + dur_to_read > total_indur) {
      warn("TRANSBEND", "This note will read off the end of the input file.\n"
                    "You might not get the envelope decay you "
                    "expect from setline.\nReduce output duration.");
      /* no exit() */
   }
#endif

   /* total number of frames to read during life of inst */
   in_frames_left = (int) (nsamps * averageInc + 0.5);

   /* to trigger first read in run() */
   inframe = RTBUFSAMPS;

   amptable = floc(1);
   if (amptable) {
      int amplen = fsize(1);
      tableset(dur, amplen, tabs);
   }
   else
      advise("TRANSBEND", "Setting phrase curve to all 1's.");

   skip = (int) (SR / (float) resetval);

   return nsamps;
}


int TRANSBEND :: run()
{
   const int out_frames = chunksamps;
   int       i, branch = 0, ibranch = 0;
   float     aamp, *outp;
   double    frac;
   const float cpsoct10 = cpsoct(10.0);

#ifdef DEBUG
   printf("out_frames: %d  in_frames_left: %d\n", out_frames, in_frames_left);
#endif

   /* If this is first call to run, allocate input buffer, which
      must persist across calls to run.
   */
   if (in == NULL)
      in = new float[inputchans * RTBUFSAMPS];

   Instrument :: run();

   aamp = amp;                  /* in case amptable == NULL */
   outp = outbuf;               /* point to inst private out buffer */

   for (i = 0; i < out_frames; i++) {
      if (--branch < 0) {
         if (amptable)
            aamp = table(cursamp, amptable, tabs) * amp;
         branch = skip;
      }
      while (get_frame) {
         if (inframe >= RTBUFSAMPS) {
            rtgetin(in, this, RTBUFSAMPS * inputchans);

            in_frames_left -= RTBUFSAMPS;
#ifdef DEBUG
            printf("READ %d frames, in_frames_left: %d\n",
                                                  RTBUFSAMPS, in_frames_left);
#endif
            inframe = 0;
         }
         oldersig = oldsig;
         oldsig = newsig;

         newsig = in[(inframe * inputchans) + inchan];

         inframe++;
         incount++;

         if (counter - (double) incount < 0.5)
            get_frame = 0;
      }

      frac = (counter - (double) incount) + 2.0;
      outp[0] = interp(oldersig, oldsig, newsig, frac) * aamp;

#ifdef DEBUG_FULL
      printf("i: %d counter: %g incount: %d frac: %g inframe: %d cursamp: %d\n",
             i, counter, incount, frac, inframe, cursamp);
      printf("interping %g, %g, %g => %g\n", oldersig, oldsig, newsig, outp[0]);
#endif

      if (outputchans == 2) {
         outp[1] = outp[0] * (1.0 - pctleft);
         outp[0] *= pctleft;
      }

      outp += outputchans;
      cursamp++;

      if (--ibranch < 0) {
		  float interval = table(cursamp, pitchtable, ptabs);
	      increment = (double) cpsoct(10.0 + interval) / cpsoct10;
          ibranch = 20;
      }

      counter += increment;         /* keeps track of interp pointer */
      if (counter - (double) incount >= -0.5)
         get_frame = 1;
   }

#ifdef DEBUG
   printf("OUT %d frames\n\n", i);
#endif

   return i;
}


Instrument *makeTRANSBEND()
{
   TRANSBEND *inst;

   inst = new TRANSBEND();
   inst->set_bus_config("TRANSBEND");

   return inst;
}


void rtprofile()
{
   RT_INTRO("TRANSBEND", makeTRANSBEND);
}


