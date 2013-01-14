/* TRANS - transpose a mono input signal using cubic spline interpolation

   p0 = output start time
   p1 = input start time
   p2 = output duration (time to end if negative)
   p3 = amplitude multiplier
   p4 = interval of transposition, in octave.pc
   p5 = input channel [optional, default is 0]
   p6 = pan (in percent-to-left form: 0-1) [optional, default is .5]

   p3 (amplitude), p4 (transposition) and p6 (pan) can receive dynamic updates
   from a table or real-time control source.

   If updating transposition, work in linear octaves, and then convert to
   octave.pc before passing to the instrument.  This avoids arithmetic errors
   that result from the fact that 7.13 is equal to 8.01 in octave.pc.
   Here's an example of how to handle the conversion.

      low = octpch(-0.05)
      high = octpch(0.03)
      transp = maketable("line", "nonorm", 1000, 0,0, 1,low, 3,high)
      transp = makeconverter(transp, "pchoct")
      TRANS(start, inskip, dur, amp, transp)

   This transposition starts at 0, moves down by a perfect fourth (-0.05),
   then up to a minor third (0.03) above the starting transposition.  The
   table is expressed in linear octaves, then converted to octave.pc by the
   call to makeconverter.

   TRANS processes only one channel at a time.

   TRANS was written by Doug Scott.
   Revised by John Gibson, 2/29/00.
   Revised for v4 by JG, 3/27/05.
*/
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <assert.h>
#include <ugens.h>
#include <mixerr.h>
#include <PField.h>
#include <Option.h>     // for fastUpdate
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


TRANS::TRANS() : Instrument()
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
}


TRANS::~TRANS()
{
   delete [] in;
}


// In fastUpdate mode, we skip doupdate() entirely, instead updating only amp,
// and only from a table.  The table can be a makegen or a PField table.  PField
// tables must be "flattened" using copytable if they are compound (e.g. passed
// through a PField filter or multiplied by a constant).  We use p[ampindex] as
// an amp multiplier, unless using a PField table, in which case there is no amp
// multiplier -- the p[ampindex] value is the first table value.   -JGG

void TRANS::initamp(float dur, double p[], int ampindex, int ampgenslot)
{
   fastUpdate = Option::fastUpdate();
   if (fastUpdate) {
      // Prefer PField table, otherwise makegen
      int tablen = 0;
      amptable = (double *) getPFieldTable(ampindex, &tablen);
      if (amptable)
         ampmult = 1.0f;
      else {
         ampmult = p[ampindex];
         amptable = floc(ampgenslot);
         if (amptable)
            tablen = fsize(ampgenslot);
      }
      if (amptable)
         tableset(SR, dur, tablen, amptabs);
      else
         amp = ampmult;
   }
   else {
      // NB: ampmult never used, first amp set in doupdate
      amptable = floc(ampgenslot);
      if (amptable) {
         int tablen = fsize(ampgenslot);
         tableset(SR, dur, tablen, amptabs);
      }
   }
}


int TRANS::init(double p[], int n_args)
{
   nargs = n_args;
   if (nargs < 5)
      return die("TRANS",
                 "Usage: TRANS(start, inskip, dur, amp, trans[, inchan, pan])");

   const float outskip = p[0];
   const float inskip = p[1];
   float dur = p[2];
   if (dur < 0.0)
      dur = -dur - inskip;
   inchan = (nargs > 5) ? (int) p[5] : 0;
   pctleft = (nargs > 6) ? p[6] : 0.5;

   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;
   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;

   if (inchan >= inputChannels()) {
      return die("TRANS", "You asked for channel %d of a %d-channel file.",
                                                  inchan, inputChannels());
   }

   // to trigger first read in run()
   inframe = RTBUFSAMPS;

   initamp(dur, p, 3, 1);

   oneover_cpsoct10 = 1.0 / cpsoct(10.0);
   if (fastUpdate)   // no transp updates
      _increment = cpsoct(10.0 + octpch(p[4])) * oneover_cpsoct10;

   return nSamps();
}

int TRANS::configure()
{
   in = new float[inputChannels() * RTBUFSAMPS];
   return in ? 0 : -1;
}

void TRANS::doupdate()
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

int TRANS::run()
{
   const int outframes = framesToRun();
   const int inchans = inputChannels();
   float *outp = outbuf;               // point to inst private out buffer

   for (int i = 0; i < outframes; i++) {
      if (--branch <= 0) {
         if (fastUpdate) {
            if (amptable)
               amp = ampmult * tablei(currentFrame(), amptable, amptabs);
         }
         else
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

         newsig = in[(inframe * inchans) + inchan];

         inframe++;
         incount++;

         if (counter - (double) incount < 0.5)
            getframe = false;
      }

      const double frac = (counter - (double) incount) + 2.0;
      outp[0] = interp(oldersig, oldsig, newsig, frac) * amp;

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

      counter += _increment;         // keeps track of interp pointer
      if (counter - (double) incount >= -0.5)
         getframe = true;
   }

   return framesToRun();
}


Instrument *makeTRANS()
{
   TRANS *inst;

   inst = new TRANS();
   inst->set_bus_config("TRANS");

   return inst;
}

extern Instrument *makeTRANS3();    // from TRANS3.cpp

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("TRANS", makeTRANS);
   RT_INTRO("TRANS3", makeTRANS3);
}
#endif

