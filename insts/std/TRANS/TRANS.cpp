/* TRANS - transpose a mono input signal using cubic spline interpolation

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

   TRANS was written by Doug Scott.
   Revised by John Gibson <johngibson@virginia.edu>, 2/29/00.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ugens.h>
#include <mixerr.h>
#include "TRANS.h"
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


TRANS :: TRANS() : Instrument()
{
   in = NULL;
   branch = 0;

   incount = 1;
   counter = 0.0;
   get_frame = 1;

   /* clear sample history */
   oldersig = 0.0;
   oldsig = 0.0;
   newsig = 0.0;
}


TRANS :: ~TRANS()
{
   delete [] in;
}


int TRANS :: init(double p[], int n_args)
{
   float outskip, inskip, dur, transp, interval, total_indur, dur_to_read;
	int rvin;

   if (n_args < 5) {
      die("TRANS", "Wrong number of args.");
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

   if (inchan >= inputchans) {
      die("TRANS", "You asked for channel %d of a %d-channel file.",
                                                       inchan, inputchans);
		return(DONT_SCHEDULE);
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
      warn("TRANS", "This note will read off the end of the input file.\n"
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
      tableset(dur, amplen, tabs);
   }
   else
      advise("TRANS", "Setting phrase curve to all 1's.");
   aamp = amp;

   skip = (int) (SR / (float) resetval);

   return nsamps;
}


int TRANS :: run()
{
   const int out_frames = chunksamps;
   int       i;
   float     *outp;
   double    frac;

#ifdef DEBUG
   printf("out_frames: %d  in_frames_left: %d\n", out_frames, in_frames_left);
#endif

   /* If this is first call to run, allocate input buffer, which
      must persist across calls to run.
   */
   if (in == NULL)
      in = new float[inputchans * RTBUFSAMPS];

   Instrument :: run();

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

      counter += increment;         /* keeps track of interp pointer */
      if (counter - (double) incount >= -0.5)
         get_frame = 1;
   }

#ifdef DEBUG
   printf("OUT %d frames\n\n", i);
#endif

   return i;
}


Instrument *makeTRANS()
{
   TRANS *inst;

   inst = new TRANS();
   inst->set_bus_config("TRANS");

   return inst;
}

extern Instrument *makeTRANS3();	// from TRANS3.C

void rtprofile()
{
   RT_INTRO("TRANS", makeTRANS);
   RT_INTRO("TRANS3", makeTRANS3);
}


