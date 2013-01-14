/* NOISE - make noise

   p0 = output start time
   p1 = duration
   p2 = amplitude
   p3 = percent of signal to left output channel [optional, default is .5]

   p2 (amplitude) and p3 (pan) can receive dynamic updates from a table or
   real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   The series of random numbers that makes the noise is affected by any
   calls to srand given in the script.  If there are no such calls, the
   random seed is 1.

   JGG <johgibso at indiana dot edu>, 24 Dec 2002, rev. 7/9/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>
#include "NOISE.h"
#include <rt.h>
#include <rtdefs.h>


NOISE :: NOISE() : Instrument()
{
   branch = 0;
}


NOISE :: ~NOISE()
{
}


int NOISE :: init(double p[], int n_args)
{
   nargs = n_args;

   float outskip = p[0];
   float dur = p[1];

   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(SR, dur, lenamp, amptabs);
   }

   skip = (int) (SR / (float) resetval);

   return nSamps();
}


int NOISE :: run()
{
   for (int i = 0; i < framesToRun(); i++) {
      if (--branch <= 0) {
         double p[nargs];
         update(p, nargs);
         amp = p[2];
         if (amparray)
            amp *= tablei(currentFrame(), amparray, amptabs);
         pctleft = nargs > 3 ? p[3] : 0.5;         // default is .5
         branch = skip;
      }

      float out[2];
      out[0] = rrand() * amp;

      if (outputChannels() == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeNOISE()
{
   NOISE *inst;

   inst = new NOISE();
   inst->set_bus_config("NOISE");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("NOISE", makeNOISE);
}
#endif
