/* TRANS3 - transpose a mono input signal using 3rd-order interpolation

   p0 = output start time
   p1 = input start time
   p2 = output duration (time to end if negative)
   p3 = amplitude multiplier
   p4 = interval of transposition, in octave.pc
   p5 = input channel [optional, default is 0]
   p6 = percent to left [optional, default is .5]

   Processes only one channel at a time.

   Assumes function table 1 is amplitude curve for the note.
   You can call setline for this.

   TRANS3 was written by Doug Scott.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ugens.h>
#include <mixerr.h>
#include "TRANS3.h"
#include <rt.h>

//#define DEBUG
//#define DEBUG_FULL

const float one = 1.0;
const float two = 2.0;
const float three = 3.0;
const float onehalf = 0.5;
const float onesixth = 0.166666666667;

inline float interp3rdOrder(float ym2, float ym1, float yp1, float yp2, float t)
{
	const float a = t + one;
	const float c = t - one;
	const float d = t - two;

	const float e = a * t;
	const float f = c * d;

	return onehalf * (a * f * ym1 - e * d * yp1) +
	       onesixth * (e * c * yp2 - t * f * ym2);
}

TRANS3 :: TRANS3() : Instrument()
{
   in = NULL;

   incount = 1;
   counter = 0.0;
   get_frame = 1;

   /* clear sample history */
   oldersig = 0.0;
   oldsig = 0.0;
   newsig = 0.0;
   newestsig = 0.0;
}


TRANS3 :: ~TRANS3()
{
   delete [] in;
}


int TRANS3 :: init(double p[], int n_args)
{
   float outskip, inskip, dur, transp, interval, total_indur, dur_to_read;
	int rvin;

   if (n_args < 5) {
      die("TRANS3", "Wrong number of args.");
		return(DONT_SCHEDULE);
	}

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   transp = p[4];
   inchan = (n_args > 5) ? (int) p[5] : 0;
   pctleft = (n_args > 6) ? p[6] : 0.5;

   if (dur < 0.0)
      dur = -dur - inskip;

   nsamps = rtsetoutput(outskip, dur, this);
   rvin = rtsetinput(inskip, this);
	if (rvin == -1) { // no input
		return(DONT_SCHEDULE);
	}

   if (inchan >= inputChannels()) {
      return die("TRANS3", "You asked for channel %d of a %d-channel file.",
												   inchan, inputChannels());
	}
   interval = octpch(transp);
   increment = (double) cpsoct(10.0 + interval) / cpsoct(10.0);
#ifdef DEBUG
   printf("increment: %g\n", increment);
#endif

#ifdef NOTYET
   total_indur = (float) m_DUR(NULL, 0);
   dur_to_read = dur * increment;
   if (inskip + dur_to_read > total_indur) {
      warn("TRANS3", "This note will read off the end of the input file.\n"
                    "You might not get the envelope decay you "
                    "expect from setline.\nReduce output duration.");
      /* no exit() */
   }
#endif

   /* total number of frames to read during life of inst */
   in_frames_left = (int) (nsamps * increment + 0.5);

   /* to trigger first read in run() */
   inframe = RTBUFSAMPS;

   amptable = floc(1);
   if (amptable) {
      int amplen = fsize(1);
      tableset(SR, dur, amplen, tabs);
   }
   else
      advise("TRANS3", "Setting phrase curve to all 1's.");

   skip = (int) (SR / (float) resetval);

   return nsamps;
}

int TRANS3::configure()
{
   in = new float[inputChannels() * RTBUFSAMPS];
   return in ? 0 : -1;
}

int TRANS3 :: run()
{
   const int out_frames = chunksamps;
   int       i, branch = 0, inChans = inputChannels();
   float     aamp, *outp;
   double    frac;

#ifdef DEBUG
   printf("out_frames: %d  in_frames_left: %d\n", out_frames, in_frames_left);
#endif

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
            rtgetin(in, this, RTBUFSAMPS * inChans);

            in_frames_left -= RTBUFSAMPS;
#ifdef DEBUG
            printf("READ %d frames, in_frames_left: %d\n",
                                                  RTBUFSAMPS, in_frames_left);
#endif
            inframe = 0;
         }
         oldersig = oldsig;
         oldsig = newsig;
		 newsig = newestsig;

         newestsig = in[(inframe * inChans) + inchan];

         inframe++;
         incount++;

         if (counter - (double) incount < 0.)
            get_frame = 0;
      }

//      frac = (counter - (double) incount) + 2.0;
      frac = (counter - (double) incount) + 1.0;
      outp[0] = interp3rdOrder(oldersig, oldsig, newsig, newestsig, frac) * aamp;

#ifdef DEBUG_FULL
      printf("i: %d counter: %g incount: %d frac: %g inframe: %d cursamp: %d\n",
             i, counter, incount, frac, inframe, cursamp);
      printf("interping %g, %g, %g, %g => %g\n", oldersig, oldsig, newsig, newestsig, outp[0]);
#endif

      if (outputchans == 2) {
         outp[1] = outp[0] * (1.0 - pctleft);
         outp[0] *= pctleft;
      }

      outp += outputchans;
      cursamp++;

      counter += increment;         /* keeps track of interp pointer */
      if (counter - (double) incount >= 0.0)
         get_frame = 1;
   }

#ifdef DEBUG
   printf("OUT %d frames\n\n", i);
#endif

   return i;
}


Instrument *makeTRANS3()
{
   TRANS3 *inst;

   inst = new TRANS3();
   inst->set_bus_config("TRANS3");

   return inst;
}

