/* FILTSWEEP - time-varying biquad filter

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier *
   p4 = ring-down duration (time to ring out filter after input stops)
   p5 = steepness (integer btw 1 and 5, inclusive) [optional, default is 1]
   p6 = balance output and input signals (0:no, 1:yes) [optional, default is 1]
   p7 = input channel [optional, default is 0]
   p8 = pan (in percent-to-left form: 0-1) [optional, default is .5]
   p9 = bypass filter (0: no, 1: yes) [optional, default is 0]
   p10 = filter center frequency (Hz) [optional; if missing, must use gen 2] **
   p11 = filter bandwidth (Hz if positive; if negative, the '-' sign acts as a
         flag to interpret the bw values as percentages (from 0 to 1) of the
         current cf.  [optional; if missing, must use gen 3] ***

   p3 (amplitude), p8 (pan), p9 (bypass), p10 (freq) and p11 (bandwidth) can
   receive dynamic updates from a table or real-time control source.

   p5 (steepness) is the number of filters to add in series.  Using more
   than 1 decreases the actual bandwidth of the total filter.  This sounds
   different from decreasing the bandwith of 1 filter using the bandwidth
   curve, described below.  (Mainly, it further attenuates sound outside the
   passband.)  If you don't set p6 (balance) to 1, you'll need to change p3
   (amp) to adjust for loss of power caused by connecting several filters
   in series.

   p6 (balance) tries to adjust the output of the filter so that it has
   the same power as the input.  This means there's less fiddling around
   with p3 (amp) to get the right amplitude (audible, but not ear-splitting)
   when the bandwidth is very low and/or steepness is > 1.  However, it has
   drawbacks: it can introduce a click at the start of the sound, it can
   cause the sound to pump up and down a bit, and it eats extra CPU time.

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p3 (amplitude), even if the latter is dynamic.

   ** If p10 is missing, you must use an old-style gen table 2 for the
   filter center frequency curve.

   *** If p11 is missing, you must use an old-style gen table 3 for the
   filter bandwidth curve.


   John Gibson (jgg9c@virginia.edu), 2/5/00; rev for v4, JGG, 7/24/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include "FILTSWEEP.h"
#include <rt.h>
#include <rtdefs.h>

#define BALANCE_WINDOW_SIZE  10


FILTSWEEP :: FILTSWEEP() : Instrument()
{
   in = NULL;
   balancer = NULL;
   branch = 0;
}


FILTSWEEP :: ~FILTSWEEP()
{
   delete [] in;
   for (int i = 0; i < nfilts; i++)
      delete filt[i];
   delete balancer;
}


int FILTSWEEP :: init(double p[], int n_args)
{
   nargs = n_args;
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   float ringdur = p[4];
   nfilts = n_args > 5 ? (int) p[5] : 1;
   do_balance = n_args > 6 ? (bool) p[6] : true;
   inchan = n_args > 7 ? (int) p[7] : 0;            // default is chan 0

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   if (inchan >= inputChannels())
      return die("FILTSWEEP", "You asked for channel %d of a %d-channel file.",
                                                      inchan, inputChannels());
   if (rtsetoutput(outskip, dur + ringdur, this) == -1)
      return DONT_SCHEDULE;
   insamps = (int) (dur * SR + 0.5);

   if (nfilts < 1 || nfilts > MAXFILTS)
      return die("FILTSWEEP",
               "Steepness (p5) must be an integer between 1 and %d.", MAXFILTS);
   for (int i = 0; i < nfilts; i++)
      filt[i] = new JGBiQuad(SR);

   if (do_balance) {
      balancer = new Balance(SR);
      balancer->setWindowSize(BALANCE_WINDOW_SIZE);
      scale = 1.0;
   }
   else
      scale = 1.0 / pow((double) nfilts, 1.8);     // just a guess; needs work

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(SR, dur, lenamp, amptabs);
   }

   if (n_args < 11) {         // no p10 center freq PField, must use gen table
      cfarray = floc(2);
      if (cfarray == NULL)
         return die("FILTSWEEP", "Either use the center frequency pfield (p10) "
                    "or make an old-style gen function in slot 2.");
      int len = fsize(2);
      tableset(SR, dur, len, cftabs);
   }

   if (n_args < 12) {         // no p11 bandwidth PField, must use gen table
      bwarray = floc(3);
      if (bwarray == NULL)
         return die("FILTSWEEP", "Either use the bandwidth pfield (p11) "
                    "or make an old-style gen function in slot 3.");
      int len = fsize(3);
      tableset(SR, dur, len, bwtabs);
   }

   return nSamps();
}


void FILTSWEEP :: doupdate()
{
   double p[12];
   update(p, 12, kPan | kBypass | kFreq | kBandwidth);

   amp = update(3, insamps);
   if (amparray)
      amp *= tablei(currentFrame(), amparray, amptabs);

   pctleft = nargs > 8 ? p[8] : 0.5;                // default is center
   bypass = nargs > 9 ? (bool) p[9] : false;        // default is no

   float newcf;
   if (nargs > 10)
      newcf = p[10];
   else
      newcf = tablei(currentFrame(), cfarray, cftabs);
   if (newcf < 0.0)
      newcf = 1.0;
   else if (newcf > SR * 0.5)
      newcf = SR * 0.5;

   float newbw;
   if (nargs > 11)
      newbw = p[11];
   else
      newbw = tablei(currentFrame(), bwarray, bwtabs);
   if (newbw < 0.0) {
      if (newbw < -1.0)
         newbw = -1.0;
      newbw *= -cf;     // percent of cf
   }
   else if (newbw < 0.1)
      newbw = 0.1;      // very small bw wreaks havoc

   if (newcf != cf || newbw != bw) {
      cf = newcf;
      bw = newbw;
      for (int j = 0; j < nfilts; j++)
         filt[j]->setFreqBandwidthAndGain(cf, bw, scale);
   }
}


int FILTSWEEP :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int FILTSWEEP :: run()
{
   const int samps = framesToRun() * inputChannels();

   if (currentFrame() < insamps)
      rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         doupdate();
         branch = getSkip();
      }

      float insig;
      if (currentFrame() < insamps)
         insig = in[i + inchan];
      else
         insig = 0.0;

      float out[2];
      out[0] = insig;
      if (!bypass) {
         for (int j = 0; j < nfilts; j++)
            out[0] = filt[j]->tick(out[0]);
         if (do_balance)
            out[0] = balancer->tick(out[0], insig);
      }

		out[0] *= amp;
      if (outputChannels() == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeFILTSWEEP()
{
   FILTSWEEP *inst;

   inst = new FILTSWEEP();
   inst->set_bus_config("FILTSWEEP");

   return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
   RT_INTRO("FILTSWEEP", makeFILTSWEEP);
}
#endif

