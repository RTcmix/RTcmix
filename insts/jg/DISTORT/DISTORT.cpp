/* DISTORT - uh, distortion

   p0  = output start time
   p1  = input start time
   p2  = input duration
   p3  = amplitude multiplier
   p4  = type of distortion (1: soft clip, 2: tube, 3: variable clip,
         4: waveshaping) [NOTE: 2 doesn't work correctly yet!]
   p5  = gain (before distortion)
   p6  = cutoff freq for low-pass filter (in cps)  (0 to disable filter)
         (The filter comes after the distortion in the signal chain.)
   p7  = input channel [optional, default is 0]
   p8  = percent to left channel [optional, default is .5]
   p9  = bypass all processing (0: no, 1: yes) [optional, default is 0]
   p10 = distortion param (only for types 3 and 4 -- must be greater
         than zero, typically as high as 100) [optional, default is 1]

   p3 (amplitude), p5 (gain), p6 (cutoff), p8 (pan), p9 (bypass), and
   p10 (distortion param) can receive dynamic updates from a table or
   real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   John Gibson (johgibso at indiana dot edu), 8/12/03, rev 7/10/04, 9/21/11.

   Distortion algorithms borrowed from others...
      soft clip and tube, from STRUM, by Charles Sullivan;
      variable clip from Laurent de Soras, via musicdsp.org;
      waveshaping from Bram de Jong, via musicdsp.org
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Ougens.h>
#include <objlib.h>
#include <Instrument.h>
#include "DISTORT.h"
#include <rt.h>
#include <rtdefs.h>


DISTORT::DISTORT()
   : usefilt(false), branch(0), param(1.0f), in(NULL), distort(NULL),
     filt(NULL), amptable(NULL)
{
}


DISTORT::~DISTORT()
{
   delete [] in;
   delete distort;
   delete filt;
   delete amptable;
}


int DISTORT::init(double p[], int n_args)
{
   nargs = n_args;
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   type = p[4];
   cutoff = n_args > 6 ? p[6] : 0.0;               // filter disabled
   inchan = n_args > 7 ? (int) p[7] : 0;           // default is chan 0

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;

   if (inchan >= inputChannels())
      return die("DISTORT", "You asked for channel %d of a %d-channel file.",
                                                      inchan, inputChannels());

   if (type == 1)
      distort = new Odistort(Odistort::SoftClip);
   else if (type == 2)
      distort = new Odistort(Odistort::SimpleTube);
   else if (type == 3)
      distort = new Odistort(Odistort::VariableClip);
   else if (type == 4)
      distort = new Odistort(Odistort::WaveShape);
   else
      return die("DISTORT", "Distortion type must be 1-4.");

   usefilt = (cutoff > 0.0);
   if (usefilt) {
      filt = new Butter(SR);
      filt->setLowPass(cutoff);
   }

   // legacy support for makegen amp
   double *function = floc(1);
   if (function) {
      int len = fsize(1);
      amptable = new TableL(SR, dur, function, len);
   }

   return nSamps();
}


int DISTORT::configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


void DISTORT::doupdate()
{
   double p[11];
   update(p, 11, kAmp | kGain | kFiltCF | kPan | kBypass | kParam);
   amp = p[3];
   if (amptable)
      amp *= amptable->tick(currentFrame(), 1.0);
   gain = p[5];
   if (usefilt) {
      if (p[6] != cutoff) {
         cutoff = p[6];
         filt->setLowPass(cutoff);
      }
   }
   pctleft = (nargs > 8) ? p[8] : 0.5;      // default is center
   bypass = (p[9] == 1.0);
   if (nargs > 10) {
      param = p[10];
		if (param < 1.0f)
			param = 1.0f;
   }
}


int DISTORT::run()
{
   const int insamps = framesToRun() * inputChannels();
   rtgetin(in, this, insamps);

   for (int i = 0; i < insamps; i += inputChannels()) {
      if (--branch <= 0) {
         doupdate();
         branch = getSkip();
      }
      float sig = in[i + inchan];
      if (!bypass) {
         sig *= (gain / 32768.0f);  // apply gain, convert range
         sig = distort->next(sig, param);
         sig *= 32768.0f;
         if (usefilt)
            sig = filt->tick(sig);
      }
      sig *= amp;
      float out[2];
      if (outputChannels() == 2) {
         out[0] = sig * pctleft;
         out[1] = sig * (1.0f - pctleft);
      }
      else
         out[0] = sig;

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeDISTORT()
{
   DISTORT *inst;

   inst = new DISTORT();
   inst->set_bus_config("DISTORT");

   return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
   RT_INTRO("DISTORT", makeDISTORT);
}
#endif
