/* TRANSBEND - perform a time-varying transpose on a mono input signal 
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

   Uses the function table specified by p4 as the transposition curve
   for the note.  The interval values in this table are expressed in
   linear octaves.  You can call makegen(2, ...) for this.  So if you
   want to gliss from a perfect 5th above to a perfect 5th below, use:
      makegen(-2, 18, 1000, 0,octpch(.07), 1,octpch(-.07))

   Created from my and John Gibson's TRANS by Doug Scott
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
   _increment = 0.0;

   /* clear sample history */
   oldersig = 0.0;
   oldsig = 0.0;
   newsig = 0.0;
}


TRANSBEND :: ~TRANSBEND()
{
   delete [] in;
}


int TRANSBEND :: init(double p[], int n_args)
{
   float outskip, inskip, dur, transp, interval = 0, total_indur, dur_to_read;
   float averageInc;
   int pgen;

   if (n_args < 5) {
      die("TRANSBEND", "Wrong number of args.");
		return(DONT_SCHEDULE);
	}

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   pgen = (int) p[4];
   inchan = (n_args > 5) ? (int) p[5] : 0;
   pctleft = (n_args > 6) ? p[6] : 0.5;

   if (dur < 0.0)
      dur = -dur - inskip;

   if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
   if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

   if (inchan >= inputChannels()) {
      return die("TRANSBEND", "You asked for channel %d of a %d-channel file.",
												   inchan, inputChannels());
	}
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
      tableset(SR, dur, plen, ptabs);
   }
   else {
      die("TRANSBEND", "Unable to load pitch curve (table %d)!", pgen);
		return(DONT_SCHEDULE);
	}

   averageInc = (double) cpsoct(10.0 + interval) / cpsoct(10.0);

#ifdef NOTYET
   total_indur = (float) m_DUR(NULL, 0);
   dur_to_read = dur * averageInc;
   if (inskip + dur_to_read > total_indur) {
      rtcmix_warn("TRANSBEND", "This note will read off the end of the input file.\n"
                    "You might not get the envelope decay you "
                    "expect from setline.\nReduce output duration.");
      /* no exit() */
   }
#endif

   /* total number of frames to read during life of inst */
   in_frames_left = (int) (nSamps() * averageInc + 0.5);

   /* to trigger first read in run() */
   inframe = RTBUFSAMPS;

   amptable = floc(1);
   if (amptable) {
      int amplen = fsize(1);
      tableset(SR, dur, amplen, tabs);
   }
   else
      rtcmix_advise("TRANSBEND", "Setting phrase curve to all 1's.");

   skip = (int) (SR / (float) resetval);

   return nSamps();
}

int TRANSBEND::configure()
{
   in = new float[inputChannels() * RTBUFSAMPS];
   return in ? 0 : -1;
}

int TRANSBEND :: run()
{
   const int out_frames = framesToRun();
   int       i, branch = 0, ibranch = 0, inChans = inputChannels();
   float     aamp, *outp;
   double    frac;
   const float cpsoct10 = cpsoct(10.0);

#ifdef DEBUG
   printf("out_frames: %d  in_frames_left: %d\n", out_frames, in_frames_left);
#endif

   aamp = amp;                  /* in case amptable == NULL */
   outp = outbuf;               /* point to inst private out buffer */

   for (i = 0; i < out_frames; i++) {
      if (--branch < 0) {
         if (amptable) {
#ifdef MAXMSP
            aamp = rtcmix_table(currentFrame(), amptable, tabs) * amp;
#else
            aamp = table(currentFrame(), amptable, tabs) * amp;
#endif
			}
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

         newsig = in[(inframe * inChans) + inchan];

         inframe++;
         incount++;

         if (counter - (double) incount < 0.5)
            get_frame = 0;
      }

      frac = (counter - (double) incount) + 2.0;
      outp[0] = interp(oldersig, oldsig, newsig, frac) * aamp;

#ifdef DEBUG_FULL
      printf("i: %d counter: %g incount: %d frac: %g inframe: %d cursamp: %d\n",
             i, counter, incount, frac, inframe, currentFrame());
      printf("interping %g, %g, %g => %g\n", oldersig, oldsig, newsig, outp[0]);
#endif

      if (outputchans == 2) {
         outp[1] = outp[0] * (1.0 - pctleft);
         outp[0] *= pctleft;
      }

      outp += outputchans;
      increment();

      if (--ibranch < 0) {
#ifdef MAXMSP
		  float interval = rtcmix_table(currentFrame(), pitchtable, ptabs);
#else
		  float interval = table(currentFrame(), pitchtable, ptabs);
#endif
	      _increment = (double) cpsoct(10.0 + interval) / cpsoct10;
          ibranch = 20;
      }

      counter += _increment;         /* keeps track of interp pointer */
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

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("TRANSBEND", makeTRANSBEND);
}
#endif

