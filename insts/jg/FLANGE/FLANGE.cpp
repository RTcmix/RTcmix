/* FLANGE - flanger, using either notch or comb filter

   p0  = output start time
   p1  = input start time
   p2  = duration
   p3  = amplitude multiplier (pre-processing)
   p4  = resonance (can be negative)
   p5  = maximum delay time (determines lowest pitch; try: 1.0 / cpspch(8.00)
   p6  = modulation depth (0 - 100%)
   p7  = modulation speed (Hz)
   p8  = wet/dry mix (0: dry --> 1: wet)  [optional; default is 0.5]
   p9  = flanger type (0 is IIR comb, 1 is FIR notch)  [optional; default is 0]
   p10 = input channel  [optional; default is 0]
   p11 = stereo spread (0 - 1)  [optional; default is 0.5]

   Assumes function table 1 is an amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses a
   flat amplitude curve. This curve, combined with the amplitude multiplier,
   affect the signal BEFORE processing.

   Assumes function table 2 holds one cycle of the modulation waveform.
   Don't let the amplitude of this waveform exceed 1 (absolute value)!
   When in doubt, use "makegen(2, 10, 1000, 1)".

   (For kicks, try max delay of 1/x, where x is < 20 Hz.)

   John Gibson (jgg9c@virginia.edu), 7/21/99.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "FLANGE.h"
#include <rt.h>
#include <rtdefs.h>


FLANGE :: FLANGE() : Instrument()
{
   in = NULL;
   zcomb = NULL;       // only one of these will be created
   znotch = NULL;
}


FLANGE :: ~FLANGE()
{
   delete [] in;
   delete zcomb;
   delete znotch;
   delete modoscil;
}


int FLANGE :: init(double p[], int n_args)
{
   float outskip, inskip, dur, maxdelay, ringdur;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   resonance = p[4];
   maxdelay = p[5];
   moddepth = p[6];
   modspeed = p[7];
   wetdrymix = (n_args > 8) ? p[8] : 0.5;
   flangetype = (int)p[9];
   inchan = (int)p[10];
   spread = (n_args > 11) ? p[11] : 0.5;

   if (flangetype == 0) {
      advise("FLANGE", "Using IIR comb filter.");
      zcomb = new ZComb(maxdelay, resonance);
   }
   else {
      advise("FLANGE", "Using FIR notch filter.");
      // takes more resonance to make the notch prominent
      znotch = new ZNotch(maxdelay, resonance * 5.0);
   }

   maxdelsamps = maxdelay * SR;            // yes, maxdelsamps is float

   if (moddepth < 0.0 || moddepth > 100.0)
      return die("FLANGE", "Modulation depth must be between 0 and 100.");
   moddepth /= 100.0;         // convert to [0, 1]

   if (wetdrymix < 0.0 || wetdrymix > 1.0)
      return die("FLANGE", "Wet/dry mix must be between 0 and 1.");

   ringdur = (resonance < 0) ? -resonance : resonance;

   nsamps = rtsetoutput(outskip, dur + ringdur, this);
   if (rtsetinput(inskip, this) != 0)
      return DONT_SCHEDULE;
   insamps = (int)(dur * SR);

   if (inchan >= inputchans)
      return die("FLANGE", "You asked for channel %d of a %d-channel file.",
                                                       inchan, inputchans);

   double *modtable = floc(2);
   if (modtable) {
      int len = fsize(2);
      modoscil = new OscilN(0.0, modtable, len);
   }
   else
      return die("FLANGE",
                 "You haven't made the modulation waveform (table 2).");

   amparray = floc(1);
   if (amparray) {
      int len = fsize(1);
      tableset(SR, dur, len, amptabs);
   }
   else
      advise("FLANGE", "Setting phrase curve to all 1's.");

   skip = (int)(SR / (float)resetval);

   return nsamps;
}


int FLANGE :: run()
{
   int   i, branch, rsamps;
   float aamp, modval, insig, fsig, delsamps, modsamps;
   float out[2];

   if (in == NULL)              /* first time, so allocate it */
      in = new float [RTBUFSAMPS * inputchans];

   rsamps = chunksamps * inputchans;

   rtgetin(in, this, rsamps);

   aamp = amp;                  /* in case amparray == NULL */

   branch = 0;
   for (i = 0; i < rsamps; i += inputchans) {
      if (--branch < 0) {
         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs) * amp;
         branch = skip;
      }
      if (cursamp < insamps)                 /* still taking input from file */
         insig = in[i + inchan] * aamp;
       
      else                                   /* in ring-down phase */
         insig = 0.0;

      /* get modulation, shifted to fall in range [0, 1]. */
      modval = modoscil->tick(modspeed, 0.5);
      modval += 0.5;

      /* compute time-varying delay tap. Note that this prevents negative
         delay, since modval * moddepth can never be > 1.
         However, user could screw us up if they set up their oscil waveform
         so that it exceeds [-1, 1]!
      */
      modsamps = maxdelsamps * (modval * moddepth);
      delsamps = maxdelsamps - modsamps;

      if (flangetype == 0)
         fsig = zcomb->tick(insig, delsamps);
      else
         fsig = znotch->tick(insig, delsamps);

      out[0] = (insig * (1.0 - wetdrymix)) + (fsig * wetdrymix);

      if (outputchans == 2) {
         out[1] = out[0] * (1.0 - spread);
         out[0] *= spread;
      }

      rtaddout(out);
      cursamp++;
   }

   return i;
}


Instrument *makeFLANGE()
{
   FLANGE *inst;

   inst = new FLANGE();
   inst->set_bus_config("FLANGE");

   return inst;
}

void
rtprofile()
{
   RT_INTRO("FLANGE", makeFLANGE);
}


