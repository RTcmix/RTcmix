/* FREEVERB - a reverberator

   This reverb instrument is based on Freeverb, by Jezar
   (http://www.dreampoint.co.uk/~jzracc/freeverb.htm).

   p0  = output start time
   p1  = input start time
   p2  = input duration
   p3  = amplitude multiplier
   p4  = room size (0-1.07143 ... don't ask)
   p5  = pre-delay time (time between dry signal and onset of reverb)
   p6  = ring-down duration
   p7  = damp (0-100%)
   p8  = dry signal level (0-100%)
   p9  = wet signal level (0-100%)
   p10 = stereo width of reverb (0-100%)

   p3 (amplitude), p4 (room size), p5 (pre-delay), p7 (damp), p8 (dry),
   p9 (wet) and p10 (stereo width) can receive dynamic updates from a table
   or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   The amplitude multiplier is applied to the input sound *before*
   it enters the reverberator.

   If you enter a room size greater than the maximum, you'll get the
   maximum amount -- which is probably an infinite reverb time.

   Input can be mono or stereo; output can be mono or stereo.

   Be careful with the dry and wet levels -- it's easy to get extreme
   clipping!

   John Gibson <johngibson@virginia.edu>, 2 Feb 2001; rev for v4, 7/11/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <Instrument.h>
#include "FREEVERB.h"
#include <rt.h>
#include <rtdefs.h>


FREEVERB :: FREEVERB() : Instrument()
{
   in = NULL;
   branch = 0;
   warn_roomsize = true;
   warn_predelay = true;
   warn_damp = true;
   warn_dry = true;
   warn_wet = true;
   warn_width = true;
}


FREEVERB :: ~FREEVERB()
{
   delete [] in;
   delete rvb;
}


int FREEVERB :: init(double p[], int n_args)
{
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   roomsize = p[4];
   predelay_time = p[5];
   ringdur = p[6];
   damp = p[7];
   dry = p[8];
   wet = p[9];
   width = p[10];

   // Keep reverb comb feedback <= 1.0
   max_roomsize = (1.0 - offsetroom) / scaleroom;
   if (roomsize < 0.0)
      return die("FREEVERB", "Room size must be between 0 and %g.",
                                                               max_roomsize);
   if (roomsize > max_roomsize) {
      roomsize = max_roomsize;
      advise("FREEVERB", "Room size cannot be greater than %g. Adjusting...",
             max_roomsize);
   }
   int predelay_samps = (int) ((predelay_time * SR) + 0.5);
   if (predelay_samps > max_predelay_samps)
      return die("FREEVERB", "Pre-delay must be between 0 and %g seconds.",
                                             (float) max_predelay_samps / SR);
   if (damp < 0.0 || damp > 100.0)
      return die("FREEVERB", "Damp must be between 0 and 100%%.");
   if (dry < 0.0 || dry > 100.0)
      return die("FREEVERB", "Dry signal level must be between 0 and 100%%.");
   if (wet < 0.0 || wet > 100.0)
      return die("FREEVERB", "Wet signal level must be between 0 and 100%%.");
   if (width < 0.0 || width > 100.0)
      return die("FREEVERB", "Width must be between 0 and 100%%.");

   if (rtsetinput(inskip, this) != 0)
      return DONT_SCHEDULE;
   nsamps = rtsetoutput(outskip, dur + ringdur, this);
   insamps = (int) (dur * SR);

   if (inputChannels() > 2)
      return die("FREEVERB", "Can't have more than 2 input channels.");
   if (outputChannels() > 2)
      return die("FREEVERB", "Can't have more than 2 output channels.");

   rvb = new revmodel();

   rvb->setroomsize(roomsize);
   rvb->setpredelay(predelay_samps);
   rvb->setdamp(damp * 0.01);
   rvb->setdry(dry * 0.01);
   rvb->setwet(wet * 0.01);
   rvb->setwidth(width * 0.01);

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(SR, dur, lenamp, amptabs);
   }

   skip = (int) (SR / (float) resetval);

   return nSamps();
}


int FREEVERB :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


inline void FREEVERB :: updateRvb(double p[])
{
   if (p[4] != roomsize) {
      roomsize = p[4];
      if (roomsize < 0.0 || roomsize > max_roomsize) {
         if (warn_roomsize) {
            warn("FREEVERB", "Room size must be between 0 and %g. Adjusting...",
                                                                  max_roomsize);
            warn_roomsize = false;
         }
         roomsize = roomsize < 0.0 ? 0.0 : max_roomsize;
      }
      rvb->setroomsize(roomsize);
   }
   if (p[5] != predelay_time) {
      predelay_time = p[5];
      int predelay_samps = (int) ((predelay_time * SR) + 0.5);
      if (predelay_samps > max_predelay_samps) {
         if (warn_predelay) {
            warn("FREEVERB", "Pre-delay must be between 0 and %g seconds. "
                             "Adjusting...", (float) max_predelay_samps / SR);
            warn_predelay = false;
         }
         predelay_samps = max_predelay_samps;
      }
      rvb->setpredelay(predelay_samps);
   }
   if (p[7] != damp) {
      damp = p[7];
      if (damp < 0.0 || damp > 100.0) {
         if (warn_damp) {
            warn("FREEVERB", "Damp must be between 0 and 100%%. Adjusting...");
            warn_damp = false;
         }
         damp = damp < 0.0 ? 0.0 : 100.0;
      }
      rvb->setdamp(damp * 0.01);
   }
   if (p[8] != dry) {
      dry = p[8];
      if (dry < 0.0 || dry > 100.0) {
         if (warn_dry) {
            warn("FREEVERB", "Dry signal level must be between 0 and 100%%. "
                                                               "Adjusting...");
            warn_dry = false;
         }
         dry = dry < 0.0 ? 0.0 : 100.0;
      }
      rvb->setdry(dry * 0.01);
   }
   if (p[9] != wet) {
      wet = p[9];
      if (wet < 0.0 || wet > 100.0) {
         if (warn_wet) {
            warn("FREEVERB", "Wet signal level must be between 0 and 100%%. "
                                                               "Adjusting...");
            warn_wet = false;
         }
         wet = wet < 0.0 ? 0.0 : 100.0;
      }
      rvb->setwet(wet * 0.01);
   }
   if (p[10] != width) {
      width = p[10];
      if (width < 0.0 || width > 100.0) {
         if (warn_width) {
            warn("FREEVERB", "Width must be between 0 and 100%%. Adjusting...");
            warn_width = false;
         }
         width = width < 0.0 ? 0.0 : 100.0;
      }
      rvb->setwidth(width * 0.01);
   }
}


int FREEVERB :: run()
{
   float *inL, *inR, *outL, *outR;

   inL = in;
   inR = inputChannels() > 1 ? in + 1 : in;
   outL = outbuf;
   outR = outputChannels() > 1 ? outbuf + 1 : outbuf;

   int samps = framesToRun() * inputChannels();

   if (currentFrame() < insamps)
      rtgetin(in, this, samps);

   // Scale input signal by amplitude multiplier and setline curve.
   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         double p[11];
         update(p, 11);
         if (currentFrame() < insamps) {  // amp is pre-effect
            amp = p[3];
            if (amparray)
               amp *= tablei(currentFrame(), amparray, amptabs);
         }
         updateRvb(p);
         branch = skip;
      }
      if (currentFrame() < insamps) {     // still taking input from file
         in[i] *= amp;
         if (inputChannels() == 2)
            in[i + 1] *= amp;
      }
      else {                              // in ringdown phase
         in[i] = 0.0;
         if (inputChannels() == 2)
            in[i + 1] = 0.0;
      }
      increment();
   }

   // Hand off to Freeverb to do the actual work.
   rvb->processreplace(inL, inR, outL, outR, chunksamps, inputChannels(),
                                                         outputChannels());

   return framesToRun();
}


Instrument *makeFREEVERB()
{
   FREEVERB *inst;

   inst = new FREEVERB();
   inst->set_bus_config("FREEVERB");

   return inst;
}

void rtprofile()
{
   RT_INTRO("FREEVERB", makeFREEVERB);
}


