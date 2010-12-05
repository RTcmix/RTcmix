/* TRANS3 - transpose a mono input signal using 3rd-order interpolation

   p0 = output start time
   p1 = input start time
   p2 = output duration (time to end if negative)
   p3 = amplitude multiplier
   p4 = interval of transposition, in octave.pc
   p5 = input channel [optional, default is 0]
   p6 = percent to left [optional, default is .5]

   p3 (amplitude), p4 (transposition) and p6 (pan) can receive dynamic updates
   from a table or real-time control source.  See TRANS for information about
   a technique for updating transposition.

   TRANS3 processes only one channel at a time.

   TRANS3 was written by Doug Scott.
   Revised for v4 by JG, 2/11/06.
*/
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
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
   branch = 0;
   transp = FLT_MAX;
   incount = 1;
   _increment = 0.0;
   counter = 0.0;
   getframe = true;

   // clear sample history
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
   nargs = n_args;
   if (nargs < 5)
      return die("TRANS3",
                 "Usage: TRANS3(start, inskip, dur, amp, trans[, inchan, pan])");

   const float outskip = p[0];
   const float inskip = p[1];
   float dur = p[2];
   if (dur < 0.0)
      dur = -dur - inskip;
   inchan = (nargs > 5) ? (int) p[5] : 0;

   if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
   if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

   if (inchan >= inputChannels()) {
      return die("TRANS3", "You asked for channel %d of a %d-channel file.",
												   inchan, inputChannels());
	}

   // to trigger first read in run()
   inframe = RTBUFSAMPS;

   oneover_cpsoct10 = 1.0 / cpsoct(10.0);

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, amptabs);
	}

   return nSamps();
}

int TRANS3::configure()
{
   in = new float[inputChannels() * RTBUFSAMPS];
   return in ? 0 : -1;
}

void TRANS3::doupdate()
{
   double p[7];
   update(p, 7);

   amp = p[3];
   if (amptable)
      amp *= tablei(currentFrame(), amptable, amptabs);
   pctleft = (nargs > 6) ? p[6] : 0.5;

   float newtransp = p[4];
   if (newtransp != transp) {
      transp = newtransp;
      _increment = cpsoct(10.0 + octpch(transp)) * oneover_cpsoct10;
#ifdef DEBUG
      printf("_increment: %g\n", _increment);
#endif
   }
}

int TRANS3::run()
{
   const int outframes = framesToRun();
   const int inchans = inputChannels();
   float *outp = outbuf;     // point to inst private out buffer
   double frac;

   for (int i = 0; i < outframes; i++) {
      if (--branch <= 0) {
         doupdate();
         branch = getSkip();
      }
      while (getframe) {
         if (inframe >= RTBUFSAMPS) {
            rtgetin(in, this, RTBUFSAMPS * inchans);
            inframe = 0;
         }
         oldersig = oldsig;
         oldsig = newsig;
         newsig = newestsig;

         newestsig = in[(inframe * inchans) + inchan];

         inframe++;
         incount++;

         if (counter - (double) incount < 0.0)
            getframe = false;
      }

//      const double frac = (counter - (double) incount) + 2.0;
      const double frac = (counter - (double) incount) + 1.0;
      outp[0] = interp3rdOrder(oldersig, oldsig, newsig, newestsig, frac) * amp;

#ifdef DEBUG_FULL
      printf("i: %d counter: %g incount: %d frac: %g inframe: %d cursamp: %d\n",
             i, counter, incount, frac, inframe, currentFrame());
      printf("interping %g, %g, %g, %g => %g\n", oldersig, oldsig, newsig, newestsig, outp[0]);
#endif

      if (outputChannels() == 2) {
         outp[1] = outp[0] * (1.0 - pctleft);
         outp[0] *= pctleft;
      }

      outp += outputChannels();
      increment();

      counter += _increment;         // keeps track of interp pointer
      if (counter - (double) incount >= 0.0)
         getframe = true;
   }

#ifdef DEBUG
   printf("OUT %d frames\n\n", i);
#endif

   return framesToRun();
}


Instrument *makeTRANS3()
{
   TRANS3 *inst;

   inst = new TRANS3();
   inst->set_bus_config("TRANS3");

   return inst;
}

