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
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <mixerr.h>
#include <Instrument.h>
#include "FLANGE.h"
#include <rt.h>
#include <rtdefs.h>

extern "C" {
   #include <ugens.h>
   extern int resetval;
}


FLANGE :: FLANGE() : Instrument()
{
   zcomb = NULL;       // only one of these will be created
   znotch = NULL;
}


FLANGE :: ~FLANGE()
{
   delete zcomb;
   delete znotch;
   delete modoscil;
}


int FLANGE :: init(float p[], short n_args)
{
   int   len;
   float outskip, inskip, dur, maxdelay, ringdur;
   float *modtable;

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
      printf("Using IIR comb filter.\n");
      zcomb = new ZComb(maxdelay, resonance);
   }
   else {
      printf("Using FIR notch filter.\n");
      // takes more resonance to make the notch prominent
      znotch = new ZNotch(maxdelay, resonance * 5.0);
   }

   maxdelsamps = maxdelay * SR;            // yes, maxdelsamps is float

   if (moddepth < 0.0 || moddepth > 100.0) {
      fprintf(stderr, "Modulation depth must be between 0 and 100.\n");
      exit(-1);
   }
   moddepth /= 100.0;         // convert to [0, 1]

   if (wetdrymix < 0.0 || wetdrymix > 1.0) {
      fprintf(stderr, "Wet/dry mix must be between 0 and 1.\n");
      exit(-1);
   }

   ringdur = (resonance < 0) ? -resonance : resonance;

   nsamps = rtsetoutput(outskip, dur + ringdur, this);
   rtsetinput(inskip, this);
   insamps = (int)(dur * SR);

   if (inchan >= inputchans) {
      fprintf(stderr, "You asked for channel %d of a %d-channel file.\n",
              inchan, inputchans);
      exit(-1);
   }

/*
   filt = new Butter;
   filt->setLowPass(cutoff);
*/

   modtable = floc(2);
   len = fsize(2);
   modoscil = new OscilN(0.0, modtable, len);

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(dur, lenamp, amptabs);
   }
   else
      printf("Setting phrase curve to all 1's\n");

   skip = (int)(SR / (float)resetval);

   return nsamps;
}


int FLANGE :: run()
{
   int   i, branch, rsamps;
   float aamp, modval, insig, fsig, delsamps, modsamps;
   float in[MAXBUF], out[2];

   Instrument :: run();

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

//    insig = filt->tick(insig);

      /* get modulation, shifted to fall in range [0, 1].
      */
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


