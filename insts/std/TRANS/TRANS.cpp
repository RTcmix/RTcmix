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
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <mixerr.h>
#include "TRANS.h"
#include <rt.h>

//#define DEBUG

extern "C" {
   #include <ugens.h>
   extern int resetval;
}

static float interp(float, float, float, float);



TRANS :: TRANS() : Instrument()
{
   in = new float[MAXBUF + (MAXCHANS * 2)];
   out = new float[MAXBUF];

   inframe = RTBUFSAMPS;
   incount = 0;
   counter = 0.0;
   getflag = 1;
   first_time = 1;

   /* clear sample history */
   oldersig = 0.0;
   oldsig = 0.0;
   newsig = 0.0;
}


TRANS :: ~TRANS()
{
   delete [] in;
   delete [] out;
}


int TRANS :: init(float p[], short n_args)
{
   float outskip, inskip, dur, transp, interval;

   if (n_args < 5) {
      cerr << "TRANS: Wrong number of args." << endl;
      return -1;
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
   rtsetinput(inskip, this);

   interval = octpch(transp);
   increment = (double) cpsoct(10.0 + interval) / cpsoct(10.0);
#ifdef DEBUG
   printf("increment: %g\n", increment);
#endif

   amptable = floc(1);
   if (amptable) {
      int amplen = fsize(1);
      tableset(p[2], amplen, tabs);
   }
   else
      printf("Setting phrase curve to all 1's\n");

   skip = (int) (SR / (float) resetval);

   return nsamps;
}


inline int min(int x, int y)
{
   if (x < y)
      return x;
   else
      return y;
}


int TRANS :: run()
{
   const int outframes = chunksamps;
   int       i;
   int       branch = 0;
   float     aamp;
   float     *outp = out;

   aamp = amp;                  /* in case amptable == NULL */

   if (first_time) {
      inframe = outframes;
      first_time = 0;
   }

   for (i = 0; i < outframes; i++) {
      if (--branch < 0) {
         if (amptable)
            aamp = table(cursamp, amptable, tabs) * amp;
         branch = skip;
      }
      while (getflag) {
         int index;

         if (inframe == outframes) {          /* time for an input buffer */
            rtgetin(in, this, inputchans * outframes);
#ifdef DEBUG
            printf("READ %d frames\n", outframes);
#endif
            inframe = 0;
         }
         oldersig = oldsig;
         oldsig = newsig;

         index = inframe * inputchans;
         newsig = in[index + inchan];

         incount++;
         inframe++;
         if (counter - (double) incount < 0.5)
            getflag = 0;
      }

      double frac = counter - (double) incount + 2.0;
      outp[0] = interp(oldersig, oldsig, newsig, frac) * aamp;
#ifdef DEBUG
      printf("i: %d counter: %g incount: %d frac: %g inframe: %d cursamp: %d\n",
             i, counter, incount, frac, inframe, cursamp);
      printf("interping %g, %g, %g\n", oldersig, oldsig, newsig);
#endif

      if (NCHANS == 2) {
         outp[1] = outp[0] * (1.0 - pctleft);
         outp[0] *= pctleft;
      }

      outp += NCHANS;
      cursamp++;

      counter += increment;     // keeps track of interp pointer
      if (counter - (double) incount >= -0.5)
         getflag = 1;
   }

   rtbaddout(out, outframes * NCHANS);
#ifdef DEBUG
   printf("OUT %d samples\n\n", i);
#endif

   return i;
}


Instrument *makeTRANS()
{
   return new TRANS();
}


void rtprofile()
{
   RT_INTRO("TRANS", makeTRANS);
}


static float interp(float y0, float y1, float y2, float t)
{
   float hy2, hy0, a, b, c;

   a = y0;
   hy0 = y0 / 2.0f;
   hy2 = y2 / 2.0f;
   b = (-3.0f * hy0) + (2.0f * y1) - hy2;
   c = hy0 - y1 + hy2;

   return (a + b * t + c * t * t);
}

