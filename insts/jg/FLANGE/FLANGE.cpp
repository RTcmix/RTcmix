/* FLANGE - flanger, using either notch or comb filter

   p0  = output start time
   p1  = input start time
   p2  = duration
   p3  = amplitude multiplier (pre-processing) *
   p4  = resonance (can be negative)
   p5  = maximum delay time (determines lowest pitch; try: 1.0 / cpspch(8.00)
   p6  = modulation depth (0 - 100%)
   p7  = modulation rate (Hz)
   p8  = wet/dry mix (0: dry --> 1: wet)  [optional; default is 0.5]
   p9  = flanger type ("IIR" is IIR comb, "FIR" is FIR notch)
         [optional; default is "iir"] **
   p10 = input channel  [optional; default is 0]
   p11 = pan (in percent-to-left form: 0-1) [optional; default is 0.5]
   p12 = ring-down duration [optional; default is resonance value]
   p13 = reference to mod. wavetable [optional; if missing, must use gen 2 ***,
         or default to internal sine wave]
         Don't let the amplitude of this waveform exceed 1 (absolute value)!

   p3 (amplitude), p4 (resonance), p6 (modulation depth), p7 (modulation rate),
   p8 (wet/dry mix), p9 (flanger type) and p11 (pan) can receive dynamic updates
   from a table or real-time control source.  p9 (flanger type) can be updated
   only when using numeric codes. **

   The point of the ring-down duration parameter (p12) is to let you control
   how long the flanger will ring after the input has stopped.  If you set p12
   to zero, then FLANGE will try to figure out the correct ring-down duration
   for you.  This will almost always be fine.  However, if resonance is dynamic,
   there are cases where FLANGE's estimate of the ring duration will be too
   short, and your sound will cut off prematurely.  Use p12 to extend the
   duration.

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p3 (amplitude), even if the latter is dynamic.

   ** You can also give numeric codes for the flanger type (0: iir, 1: fir).
   These can be changed during a note.  If you give the string version, you
   can't change types during a note.

   *** If p13 is missing, you must use an old-style gen table 2 for the
   modulator waveform.  [added default sine wave, BGG 6/2012]


   John Gibson (johgibso at indiana dot edu), 7/21/99; rev for v4, JGG, 7/24/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include "FLANGE.h"
#include <rt.h>
#include <rtdefs.h>
#include <float.h>   // for FLT_MAX

// takes more resonance to make the notch prominent
#define NOTCH_RESONANCE_FACTOR 5.0

FLANGE :: FLANGE() : Instrument()
{
   in = NULL;
   branch = 0;
   flangetype_was_string = false;
	ownModtable = false;
}


FLANGE :: ~FLANGE()
{
   delete [] in;
   delete zcomb;
   delete znotch;
   delete modoscil;
	if (ownModtable)
		delete [] modtable;
}


inline int _string_to_flangecode(const char *str)
{
   if (strcasecmp(str, "IIR") == 0)
      return 0;
   else if (strcasecmp(str, "FIR") == 0)
      return 1;
   return -1;
}

int FLANGE :: getFlangeType(bool trystring)
{
   double index = (double) currentFrame() / nSamps();
   const PField &field = getPField(9);

   // must try int first, since a valid code cast to char * will crash strcmp
   int code = field.intValue(index);
   if ((code < 0 || code > 1) && trystring) {
      const char *str = field.stringValue(index);
      code = _string_to_flangecode(str);   // -1 if no match
      if (code != -1)
         flangetype_was_string = true;
   }

   return code;
}


int FLANGE :: init(double p[], int n_args)
{
   nargs = n_args;
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   resonance = p[4];
   float maxdelay = p[5];
   moddepth = p[6];
   wetdrymix = (n_args > 8) ? p[8] : 0.5;
   inchan = (int) p[10];
   float ringdur = p[12];

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   insamps = (int) (dur * SR + 0.5);
   if (inchan >= inputChannels())
      return die("FLANGE", "You asked for channel %d of a %d-channel file.",
                                                    inchan, inputChannels());
   if (ringdur == 0.0)
      ringdur = (resonance < 0.0) ? -resonance : resonance;
   if (rtsetoutput(outskip, dur + ringdur, this) == -1)
      return DONT_SCHEDULE;

   if (n_args > 9) {
      flangetype = getFlangeType(true);
      if (flangetype == -1)
         die("FLANGE", "Flanger type must be either \"IIR\" or \"FIR\".");
   }
   else
      flangetype = 0;

   zcomb = new ZComb(SR, maxdelay, resonance);
   znotch = new ZNotch(SR, maxdelay, resonance * NOTCH_RESONANCE_FACTOR);

   maxdelsamps = maxdelay * SR;            // yes, maxdelsamps is float

   if (moddepth < 0.0 || moddepth > 100.0)
      return die("FLANGE", "Modulation depth must be between 0 and 100.");
   moddepth *= 0.01;       // convert to [0, 1]

   if (wetdrymix < 0.0 || wetdrymix > 1.0)
      return die("FLANGE", "Wet/dry mix must be between 0 and 1.");

   modtable = NULL;
   int tablelen = 0;
	if (n_args > 13) {      // handle table coming in as optional p13 TablePField
		modtable = (double *) getPFieldTable(13, &tablelen);
	}
	if (modtable == NULL) {
		modtable = floc(2);
 		if (modtable)
			tablelen = fsize(2);
		else {
			rtcmix_warn("FLANGE", "No modulation wavetable specified, so using sine wave.");
			tablelen = 1024;
			modtable = new double [tablelen];
			ownModtable = true;
			const double twopi = M_PI * 2.0;
			for (int i = 0; i < tablelen; i++)
				modtable[i] = sin(twopi * ((double) i / tablelen));
		}
	}
   modoscil = new OscilL(SR, 0.0, modtable, tablelen);

   amparray = floc(1);
   if (amparray) {
      int len = fsize(1);
      tableset(SR, dur, len, amptabs);
   }

   return nSamps();
}


void FLANGE :: doupdate()
{
   double p[12];
   update(p, 12, kResonance | kModDepth | kModRate | kWetDry | kType | kPan);
   amp = update(3, insamps);
   if (amparray)
      amp *= tablei(currentFrame(), amparray, amptabs);

   // If init type spec was string, we don't allow type updates.
   if (!flangetype_was_string && nargs > 9) {
      int newtype = getFlangeType(false);
      if (newtype == -1)
         newtype = 0;
      if (newtype != flangetype) {
         flangetype = newtype;
         resonance = -FLT_MAX;         // force reverb time update below
      }
   }

   if (p[4] != resonance) {
      resonance = p[4];
      if (flangetype == 0)
         zcomb->setReverbTime(resonance);
      else
         znotch->setScaler(resonance * NOTCH_RESONANCE_FACTOR);
   }

   float rawmoddepth = p[6];
   if (rawmoddepth < 0.0)
      rawmoddepth = 0.0;
   else if (rawmoddepth > 100.0)
      rawmoddepth = 100.0;
   moddepth = rawmoddepth * 0.01;         // convert to [0, 1]

   modrate = p[7];

   wetdrymix = p[8];
   if (wetdrymix < 0.0)
      wetdrymix = 0.0;
   else if (wetdrymix > 1.0)
      wetdrymix = 1.0;

   pctleft = (nargs > 11) ? p[11] : 0.5;
}


int FLANGE :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int FLANGE :: run()
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
      if (currentFrame() < insamps)          // still taking input
         insig = in[i + inchan] * amp;
      else                                   // in ring-down phase 
         insig = 0.0;

      // Get modulation, shifted to fall in range [0, 1].
      float modval = modoscil->tick(modrate, 0.5);
      modval += 0.5;

      // Compute time-varying delay tap.  Note that this prevents negative
      // delay, since modval * moddepth can never be > 1.
      // However, user could screw us up if they set up their oscil waveform
      // so that it exceeds [-1, 1]!

      float modsamps = maxdelsamps * (modval * moddepth);
      float delsamps = maxdelsamps - modsamps;

      float fsig;
      if (flangetype == 0)
         fsig = zcomb->tick(insig, delsamps);
      else
         fsig = znotch->tick(insig, delsamps);

      float out[2];
      out[0] = (insig * (1.0 - wetdrymix)) + (fsig * wetdrymix);

      if (outputChannels() == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeFLANGE()
{
   FLANGE *inst;

   inst = new FLANGE();
   inst->set_bus_config("FLANGE");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("FLANGE", makeFLANGE);
}
#endif
