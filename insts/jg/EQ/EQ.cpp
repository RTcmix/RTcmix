/* EQ - equalizer instrument (peak/notch, shelving and high/low pass types)

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = EQ type (0: low pass, 1: high pass, 2: low shelf, 3: high shelf,
        4: peak/notch)
   p5 = input channel [optional, default is 0]
   p6 = percent of signal to left output channel [optional, default is .5]
   p7 = bypass filter (0: no, 1: yes) [optional, default is 0]

   Function tables:
      1  amplitude curve (or use setline)
         If no setline or function table 1, uses flat amplitude curve.
      2  frequency (Hz)
      3  Q (with values from 0.5 to 10.0, roughly)
      4  gain (dB) [shelf and peak/notch only]

   John Gibson <johgibso at indiana dot edu>, 7 Dec 2003

   Based on formulas by Robert Bristow-Johnson ("Audio-EQ-Cookbook") and code
   by Tom St Denis <tomstdenis.home.dhs.org> (see musicdsp.org)
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>
#include "EQ.h"
#include <rt.h>
#include <rtdefs.h>
#include <float.h>   /* for FLT_MIN, FLT_MAX */


EQ :: EQ() : Instrument()
{
   in = NULL;
   amp_table = NULL;
   freq_table = NULL;
   q_table = NULL;
   gain_table = NULL;
   branch = 0;
   prev_freq = prev_Q = prev_gain = -FLT_MAX;
}


EQ :: ~EQ()
{
   delete [] in;
   delete amp_table;
   delete freq_table;
   delete q_table;
   delete gain_table;
   delete eq;
}


int EQ :: init(float p[], int n_args)
{
   float outskip, inskip, dur;
   float *function;
   int   type, rvin;
   const float ringdur = 0.1;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   type = (int) p[4];
   inchan = n_args > 5 ? (int) p[5] : 0;           /* default is chan 0 */
   pctleft = n_args > 6 ? p[6] : 0.5;              /* default is .5 */
   bypass = n_args > 7 ? (int) p[7] : 0;           /* default is no */

   rvin = rtsetinput(inskip, this);
	if (rvin == -1) { // no input
		return(DONT_SCHEDULE);
	}
   nsamps = rtsetoutput(outskip, dur + ringdur, this);
   insamps = (int)(dur * SR + 0.5);

   if (inchan >= inputChannels()) {
      die("EQ", "You asked for channel %d of a %d-channel file.",
                                                   inchan, inputChannels());
		return(DONT_SCHEDULE);
	}

   switch (type) {
      case 0:
         eqtype = EQLowPass;
         break;
      case 1:
         eqtype = EQHighPass;
         break;
      case 2:
         eqtype = EQLowShelf;
         break;
      case 3:
         eqtype = EQHighShelf;
         break;
      case 4:
         eqtype = EQPeaking;
         break;
   }

   function = floc(1);
   if (function) {
      int len = fsize(1);
      amp_table = new TableL(dur, function, len);
   }
   else {
      advise("EQ", "Setting amplitude curve to all 1's.");
      aamp = amp;
   }

   function = floc(2);
   if (function) {
      int len = fsize(2);
      freq_table = new TableL(dur, function, len);
   }
   else {
      die("EQ", "You haven't made the EQ frequency function (table 2).");
		return(DONT_SCHEDULE);
	}

   function = floc(3);
   if (function) {
      int len = fsize(3);
      q_table = new TableL(dur, function, len);
   }
   else {
      die("EQ", "You haven't made the EQ Q function (table 3).");
		return(DONT_SCHEDULE);
	}

   if (eqtype != EQLowPass && eqtype != EQHighPass) {
      function = floc(4);
      if (function) {
         int len = fsize(4);
         gain_table = new TableL(dur, function, len);
      }
      else {
         die("EQ", "You haven't made the EQ gain function (table 4).");
			return(DONT_SCHEDULE);
		}
   }
   else
      gain = 0.0;

   eq = new Equalizer(eqtype);

   skip = (int) (SR / (float) resetval);

   return nsamps;
}


int EQ :: run()
{
   int   i, samps;
   float insig, outsig;
   float out[2];

   if (in == NULL)
      in = new float [RTBUFSAMPS * inputChannels()];

   Instrument::run();

   samps = framesToRun() * inputChannels();
   if (currentFrame() < insamps)
      rtgetin(in, this, samps);

   for (i = 0; i < samps; i += inputChannels()) {
      if (--branch < 0) {
         if (amp_table)
            aamp = amp_table->tick(currentFrame(), amp);
         freq = freq_table->tick(currentFrame(), 1.0);
         Q = q_table->tick(currentFrame(), 1.0);
         if (Q <= 0.0)
            Q = FLT_MIN;
         if (gain_table)
            gain = gain_table->tick(currentFrame(), 1.0);
         if (freq != prev_freq || Q != prev_Q || gain != prev_gain)
            eq->setCoeffs(freq, Q, gain);
         prev_freq = freq;
         prev_Q = Q;
         prev_gain = gain;
         branch = skip;
      }

      if (currentFrame() < insamps)
         insig = in[i + inchan];
      else
         insig = 0.0;

      if (bypass)
         outsig = insig;
      else
         outsig = (float) eq->tick((double) insig);
      out[0] = outsig * aamp;

      if (outputChannels() == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeEQ()
{
   EQ *inst;

   inst = new EQ();
   inst->set_bus_config("EQ");

   return inst;
}

void rtprofile()
{
   RT_INTRO("EQ", makeEQ);
}

