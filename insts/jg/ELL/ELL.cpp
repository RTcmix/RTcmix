/* ELL - elliptical filter

   First call ellset() to set up the filter.

     for lowpass filter:
       p0  passband cutoff (in cps) - this will be lower than the stopband
       p1  stopband cutoff (in cps)
       p2  set this to zero

     for hipass filter:
       p0  passband cutoff (in cps) - this will be higher than the stopband
       p1  stopband cutoff (in cps)
       p2  set this to zero

     for bandpass filter:
       p0  lower passband cutoff (in cps)
       p1  higher passband cutoff (in cps)
       p2  stopband cutoff, either higher or lower (in cps)
           (higher seems more reliable)

     for all three types:
       p3  ripple (in db)  [try 0.2]
       p4  attenuation at stopband (in db)  [try 90 for a steep filter]

   Then call ELL() to perform the filtration.

     p0  output start time
     p1  input start time
     p2  duration (not input end time)
     p3  amplitude multiplier
     p4  ring-down duration
     p5  input channel [optional]
     p6  stereo spread [optional]

   Assumes function table 1 holds amplitude envelope. (Or you can just use
   setline.) If this function table is empty, uses flat envelope.


   NOTES:

   <ripple> controls the amount of ringing in the filter. A small ripple
   designs the filter to minimize ringing, whereas a large ripple (c. 20db)
   causes the filter to ring very noticeably. A large ripple can sound
   good with a tight bandpass, producing a fairly clear pitch.

   The filter design program (invoked by ellset) sometimes can't fulfill
   the design criteria -- a particular combination of cutoff freqs, ripple
   and attenuation. If this happens, the program will die with a "Filter
   design failed!" message, instead of running the job. If you ask for a
   very steep cutoff and very little ripple, you may see this.
*/
#include <stdio.h>
#include <stdlib.h>
#include <mixerr.h>
#include <Instrument.h>
#include "ELL.h"
#include <rt.h>
#include <rtdefs.h>

extern "C" {
   #include <ugens.h>
   extern int resetval;
   extern int get_nsections(void);
   extern int ellpset(EllSect [], float *);
   extern float ellipse(float, int, EllSect [], float);
}


ELL::ELL() : Instrument()
{
   for (int n = 0; n < MAXCHANS; n++)
      es[n] = NULL;
}


ELL::~ELL()
{
   for (int n = 0; n < MAXCHANS; n++)
      delete [] es[n];
}


int ELL::init(float p[], short n_args)
{
   int   n;
   float outskip, inskip, dur, ringdur;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   ringdur = p[4];

   rtsetinput(inskip, this);
   nsamps = rtsetoutput(outskip, dur + ringdur, this);
   insamps = (int)(dur * SR);

   /* If user passed something for inchan or spread, check it.
      Otherwise, mark the variable so that we ignore it below.
   */
   if (n_args > 5) {
      inchan = (int)p[5];
      if (inchan >= inputchans) {
         fprintf(stderr, "You asked for channel %d of a %d-channel file.\n",
                         inchan, inputchans);
         exit(1);
      }
   }
   else
      inchan = -1;

   if (n_args > 6) {
      spread = p[6];
      if (spread < 0.0 || spread > 1.0) {
         fprintf(stderr, "spread must be between 0 and 1 (inclusive)\n");
         exit(1);
      }
   }
   else
      spread = -1.0;

   /* Set up for the various channel possibilities. */

   if (inchan == -1) {
      if (inputchans == 1)
         inchan = 0;
      else if (inputchans != NCHANS) {
         fprintf(stderr,
                 "Input and output files have differing numbers of channels,\n"
                 "so you have to specify 1 input channel.\n");
         exit(1);
      }
   }

   /* <spread> relevant only when output is stereo and there's 1 input chan. */
   if (NCHANS == 2 && inchan != -1) {
      if (spread == -1.0)
         spread = 0.5;                     /* just set it to middle */
   }
   else if (spread != -1.0) {
      printf("Note: spread ignored unless output is stereo and "
             "there's 1 input channel.\n");
      spread = -1.0;
   }

   nsects = get_nsections();
   if (nsects == 0) {
      fprintf(stderr, "You haven't called ellset to specify filter!\n");
      exit(1);
   }
   if (inchan == -1) {                     /* use all input chans */
      for (n = 0; n < inputchans; n++) {
         es[n] = new EllSect[nsects];
         ellpset(es[n], &xnorm);
      }
   }
   else {
      es[inchan] = new EllSect[nsects];
      ellpset(es[inchan], &xnorm);
   }
   printf("Filter: %d sections, normalization factor: %.9f\n", nsects, xnorm);

   amptable = floc(1);
   if (amptable) {
      int amplen = fsize(1);
      tableset(dur, amplen, amptabs);
   }
   else
      printf("Setting phrase curve to all 1's\n");

   skip = (int)(SR / (float)resetval);

   return nsamps;
}


int ELL::run()
{
   int   i, n, branch, rsamps;
   float aamp, insig;
   float in[2 * MAXBUF], out[2];

   rsamps = chunksamps * inputchans;

   rtgetin(in, this, rsamps);

   aamp = amp;                  /* in case amptable == NULL */

   branch = 0;
   for (i = 0; i < rsamps; i += inputchans) {
      if (cursamp < insamps) {
         if (--branch < 0) {
            if (amptable)
               aamp = tablei(cursamp, amptable, amptabs) * amp;
            branch = skip;
         }
         if (inchan == -1)                    /* use all input chans */
            for (n = 0; n < inputchans; n++)
               out[n] = ellipse(in[i + n] * aamp, nsects, es[n], xnorm);
         else
            insig = in[i + inchan] * aamp;
      }
      else {
         if (inchan == -1)                    /* use all input chans */
            for (n = 0; n < inputchans; n++)
               out[n] = ellipse(0.0, nsects, es[n], xnorm);
         else
            insig = 0.0;
      }
      if (inchan != -1) {                     /* only one input chan */
         float val = ellipse(insig, nsects, es[inchan], xnorm);
         if (NCHANS == 2) {                   /* then use spread */
            out[0] = val * spread; 
            out[1] = val * (1.0 - spread);
         }
         else {
            for (n = 0; n < NCHANS; n++)
               out[n] = val;
         }
      }
      rtaddout(out);
      cursamp++;
   }

   return i;
}


Instrument *makeELL()
{
   ELL *inst;

   inst = new ELL();
   return inst;
}


void rtprofile()
{
   RT_INTRO("ELL", makeELL);
}

