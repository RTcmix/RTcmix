/* NOISE - make noise

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   JGG <johgibso@indiana.edu>, 24 Dec 2002
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


int NOISE :: init(float p[], int n_args)
{
   float outskip, dur;

   outskip = p[0];
   dur = p[1];
   amp = p[2];
   pctleft = n_args > 3 ? p[3] : 0.5;                /* default is .5 */

   nsamps = rtsetoutput(outskip, dur, this);

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(dur, lenamp, amptabs);
   }
   else
      advise("NOISE", "Setting phrase curve to all 1's.");

   skip = (int) (SR / (float) resetval);

   aamp = amp;                  /* in case amparray == NULL */

   return nsamps;
}


int NOISE :: run()
{
   int   i;
   float out[2];

   Instrument::run();

   for (i = 0; i < FramesToRun(); i++) {
      if (--branch < 0) {
         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs) * amp;
         branch = skip;
      }

      out[0] = rrand() * aamp;

      if (OutputChannels() == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      increment();
   }

   return FramesToRun();
}


Instrument *makeNOISE()
{
   NOISE *inst;

   inst = new NOISE();
   inst->set_bus_config("NOISE");

   return inst;
}


void rtprofile()
{
   RT_INTRO("NOISE", makeNOISE);
}

