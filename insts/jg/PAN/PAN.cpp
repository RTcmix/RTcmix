/* PAN - simple mixing instrument that can use constant power panning

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier *
   p4 = input channel [optional, default is 0]
   p5 = 0: use constant-power panning, 1: don't use it [optional; default is 0]
   p6 = pan (in percent-to-left form: 0-1) [optional; if missing, must use
        gen 2] **

   p3 (amplitude), p5 (pan mode) and p6 (pan) can receive dynamic updates from
   a table or real-time control source.

   By default, PAN uses "constant-power" panning to prevent a sense of lost
   power when the pan location moves toward the center.  Sometimes this causes
   jerkey panning motion near hard left/right, so you can defeat it by
   by passing 1 as p5.

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p3 (amplitude), even if the latter is dynamic.

   ** If p6 is missing, you must use an old-style gen table 2 for the
   panning curve.


   John Gibson (jgg9c@virginia.edu), 1/26/00; rev for v4, JGG, 7/24/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include "PAN.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG


PAN :: PAN() : Instrument()
{
   in = NULL;
   panarray = NULL;
   branch = 0;
   prevpan = -1.0;
}


PAN :: ~PAN()
{
   delete [] in;
}


int PAN :: init(double p[], int n_args)
{
   nargs = n_args;
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   inchan = n_args > 4 ? (int) p[4] : 0;                    // default is chan 0

   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;
   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;

   if (outputChannels() != 2)
      return die("PAN", "Output must be stereo.");

   if (inchan >= inputChannels())
      return die("PAN", "You asked for channel %d of a %d-channel file.",
                                                   inchan, inputChannels());

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(SR, dur, lenamp, amptabs);
   }

   if (n_args < 7) {          // no p6 pan PField, must use gen table
      panarray = floc(2);
      if (panarray == NULL)
         return die("PAN", "Either use the pan pfield (p6) "
                    "or make an old-style gen function in slot 2.");
      int len = fsize(2);
      tableset(SR, dur, len, pantabs);
   }

   skip = (int) (SR / (float) resetval);

   return nSamps();
}


void PAN :: doupdate()
{
   double p[7];
   update(p, 7, kAmp | kUseConstPower | kPan);

   amp = p[3];
   if (amparray)
      amp *= tablei(currentFrame(), amparray, amptabs);

   bool use_constant_power = nargs > 5 ? !(bool) p[5] : true; // default is yes

   float newpan;
   if (nargs > 6)
      newpan = p[6];
   else
      newpan = tablei(currentFrame(), panarray, pantabs);
   if (newpan < 0.0)
      newpan = 0.0;
   else if (newpan > 1.0)
      newpan = 1.0;

   if (newpan != prevpan) {
      prevpan = newpan;
      if (use_constant_power) {
         pan[0] = (float) sqrt((double) newpan);
         pan[1] = (float) sqrt(1.0 - (double) newpan);
      }
      else {
         pan[0] = newpan;
         pan[1] = 1.0 - newpan;
      }
#ifdef DEBUG
      rtcmix_advise("PAN", "newpan=%f pan[0]=%f pan[1]=%f (tot=%f)",
                           newpan, pan[0], pan[1], pan[0] + pan[1]);
#endif
   }
}


int PAN :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int PAN :: run()
{
   const int samps = framesToRun() * inputChannels();

   rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         doupdate();
         branch = skip;
      }
      float insig = in[i + inchan] * amp;

      float out[2];
      out[0] = insig * pan[0];
      out[1] = insig * pan[1];

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makePAN()
{
   PAN *inst;

   inst = new PAN();
   inst->set_bus_config("PAN");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("PAN", makePAN);
}
#endif
