/* BUTTER - time-varying Butterworth filters

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = type of filter (1: lowpass, 2: highpass, 3: bandpass, 4: bandreject)
   p5 = steepness (> 0) [optional, default is 1]
   p6 = balance output and input signals (0:no, 1:yes) [optional, default is 1]
   p7 = input channel [optional, default is 0]
   p8 = percent to left channel [optional, default is .5]
   p9 = bypass filter (0: no, 1: yes) [optional, default is 0]

   p5 (steepness) is just the number of filters to add in series.  Using more
   than 1 steepens the slope of the filter.  If you don't set p6 (balance)
   to 1, you'll need to change p3 (amp) to adjust for loss of power caused
   by connecting several filters in series.  Guard your ears!

   p6 (balance) tries to adjust the output of the filter so that it has
   the same power as the input.  This means there's less fiddling around
   with p3 (amp) to get the right amplitude when steepness is > 1.  However,
   it has drawbacks: it can introduce a click at the start of the sound, it
   can cause the sound to pump up and down a bit, and it eats extra CPU time.

   Here are the function table assignments:

      1: amplitude curve
      2: cutoff (or center) frequency curve, described by time,cf pairs
         (e.g., gen 18)
      3: bandwidth curve, described by time,bw pairs [only for bandpass and
         bandreject types].  If positive, bandwidth is in Hz; if negative,
         the '-' sign acts as a flag to interpret the bw values as percentages
         (from 0 to 1) of the current cf.

   John Gibson (johgibso at indiana dot edu), 12/1/01.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "BUTTER.h"
#include <rt.h>
#include <rtdefs.h>


#define BALANCE_WINDOW_SIZE  10


BUTTER :: BUTTER() : Instrument()
{
   in = NULL;
   balancer = NULL;              // might not use
   curcf = 0.0;
   curbw = 0.0;
   branch = 0;
}


BUTTER :: ~BUTTER()
{
   delete [] in;
   for (int i = 0; i < nfilts; i++)
      delete filt[i];
   if (balancer)
      delete balancer;
}


int BUTTER :: init(double p[], int n_args)
{
   float outskip, inskip, dur;
   const float ringdur = 0.1;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   type = (FiltType)p[4];
   nfilts = n_args > 5 ? (int)p[5] : 1;
   do_balance = n_args > 6 ? (int)p[6] : 1;
   inchan = n_args > 7 ? (int)p[7] : 0;             /* default is chan 0 */
   pctleft = n_args > 8 ? p[8] : 0.5;               /* default is center */
   bypass = n_args > 9 ? (int) p[9] : 0;            /* default is no */

   if (rtsetinput(inskip, this) != 0)
      return DONT_SCHEDULE;
   nsamps = rtsetoutput(outskip, dur + ringdur, this);
   insamps = (int)(dur * SR);

   if (inchan >= inputchans)
      return die("BUTTER", "You asked for channel %d of a %d-channel file.",
                                                         inchan, inputchans);
   if (nfilts < 1 || nfilts > MAXFILTS)
      return die("BUTTER",
                 "Steepness (p5) must be an integer between 1 and %d.",
                 MAXFILTS);
   if (type != LowPass && type != HighPass && type != BandPass
         && type != BandReject)
      return die("BUTTER", "Filter type must be 1 (lowpass), 2 (highpass), "
                 "3 (bandpass) or 4 (bandreject).");

   for (int i = 0; i < nfilts; i++)
      filt[i] = new Butter();

   if (do_balance) {
      balancer = new Balance();
      balancer->setWindowSize(BALANCE_WINDOW_SIZE);
      scale = 1.0;
   }
   else
      scale = 1.0 / pow((double)nfilts, 1.8);     // just a guess; needs work

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(dur, lenamp, amptabs);
   }
   else
      advise("BUTTER", "Setting phrase curve to all 1's.");

   cfarray = floc(2);
   if (cfarray) {
      int lencf = fsize(2);
      tableset(dur, lencf, cftabs);
   }
   else
      return die("BUTTER",
                 "You haven't made the cutoff frequency function (table 2).");

   if (type == BandPass || type == BandReject) {
      bwarray = floc(3);
      if (bwarray) {
         int lenbw = fsize(3);
         tableset(dur, lenbw, bwtabs);
      }
      else
         return die("BUTTER",
                    "You haven't made the bandwidth function (table 3).");
   }

   skip = (int)(SR / (float)resetval);

   aamp = amp * scale;        /* in case amparray == NULL */

   return nsamps;
}


int BUTTER :: run()
{
   int   i, j, rsamps;
   float cf=0., bw=0., insig;
   float out[2];

   if (in == NULL)            /* first time, so allocate it */
      in = new float [RTBUFSAMPS * inputchans];

   rsamps = chunksamps * inputchans;

   if (cursamp < insamps)
      rtgetin(in, this, rsamps);

   for (i = 0; i < rsamps; i += inputchans) {
      if (--branch < 0) {
         cf = tablei(cursamp, cfarray, cftabs);
         if (cf <= 0.0)
            cf = 1.0;
         if (cf > SR * 0.5)
            cf = SR * 0.5;
         if (type == LowPass && cf != curcf) {
            for (j = 0; j < nfilts; j++)
               filt[j]->setLowPass(cf);
            curcf = cf;
         }
         else if (type == HighPass && cf != curcf) {
            for (j = 0; j < nfilts; j++)
               filt[j]->setHighPass(cf);
            curcf = cf;
         }
         else if (type == BandPass) {
            bw = tablei(cursamp, bwarray, bwtabs);
            if (bw < 0)
               bw *= -cf;     /* percent of cf */
            if (cf != curcf || bw != curbw) {
               for (j = 0; j < nfilts; j++)
                  filt[j]->setBandPass(cf, bw);
               curcf = cf;
               curbw = bw;
            }
         }
         else if (type == BandReject) {
            bw = tablei(cursamp, bwarray, bwtabs);
            if (bw < 0)
               bw *= -cf;
            if (cf != curcf || bw != curbw) {
               for (j = 0; j < nfilts; j++)
                  filt[j]->setBandReject(cf, bw);
               curcf = cf;
               curbw = bw;
            }
         }

         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs) * amp;

         branch = skip;
      }
      if (cursamp < insamps)
         insig = in[i + inchan] * aamp;
      else
         insig = 0.0;

      out[0] = insig;
      if (!bypass) {
         for (j = 0; j < nfilts; j++)
            out[0] = filt[j]->tick(out[0]);
         if (do_balance)
            out[0] = balancer->tick(out[0], insig);
      }

      if (outputchans == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      cursamp++;
   }

   return i;
}


Instrument *makeBUTTER()
{
   BUTTER *inst;

   inst = new BUTTER();
   inst->set_bus_config("BUTTER");

   return inst;
}


void
rtprofile()
{
   RT_INTRO("BUTTER", makeBUTTER);
}

