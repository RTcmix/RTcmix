/* BUTTER - time-varying Butterworth filters

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier *
   p4 = type of filter ("lowpass", "highpass", "bandpass", "bandreject") **
   p5 = steepness (> 0) [optional, default is 1]
   p6 = balance output and input signals (0:no, 1:yes) [optional, default is 1]
   p7 = input channel [optional, default is 0]
   p8 = pan (in percent-to-left form: 0-1) [optional, default is .5]
   p9 = bypass filter (0: no, 1: yes) [optional, default is 0]
   p10 = filter frequency (Hz) [optional; if missing, must use gen 2] ***
   p11 = filter bandwidth for bandpass/reject types (Hz if positive;
         if negative, the '-' sign acts as a flag to interpret the bw values
         as percentages (from 0 to 1) of the current cf.
         [optional; if missing, must use gen 3] ****

   p3 (amplitude), p4 (type), p8 (pan), p9 (bypass), p10 (freq) and 
   p11 (bandwidth) can receive dynamic updates from a table or real-time
   control source.  p4 (type) can be updated only when using numeric codes. **

   p5 (steepness) is just the number of filters to add in series.  Using more
   than 1 steepens the slope of the filter.  If you don't set p6 (balance)
   to 1, you'll need to change p3 (amp) to adjust for loss of power caused
   by connecting several filters in series.  Guard your ears!

   p6 (balance) tries to adjust the output of the filter so that it has
   the same power as the input.  This means there's less fiddling around
   with p3 (amp) to get the right amplitude when steepness is > 1.  However,
   it has drawbacks: it can introduce a click at the start of the sound, it
   can cause the sound to pump up and down a bit, and it eats extra CPU time.

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p3 (amplitude), even if the latter is dynamic.

   ** You can also give numeric codes for the filter type (1: lowpass,
   2: highpass, 3: bandpass, 4: bandreject).  These can be changed during a
   note.  If you give the string version, you can't change types during a note.

   *** If p10 is missing, you must use an old-style gen table 2 for the
   filter frequency curve.

   **** If p11 is missing, and filter type is bandpass or bandreject, then you
   must use an old-style gen table 3 for the filter bandwidth curve.


   John Gibson (johgibso at indiana dot edu), 12/1/01; rev for v4, JGG, 7/24/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include "BUTTER.h"
#include <rt.h>
#include <rtdefs.h>
#include <float.h>   // for FLT_MAX

#define BALANCE_WINDOW_SIZE  10


BUTTER :: BUTTER() : Instrument()
{
   in = NULL;
   cfarray = NULL;
   bwarray = NULL;
   cf = bw = -FLT_MAX;
   branch = 0;
   filttype_was_string = false;
}


BUTTER :: ~BUTTER()
{
   delete [] in;
   for (int i = 0; i < nfilts; i++)
      delete filt[i];
   delete balancer;
}


static const char *_filttype_name[] = {
   "lowpass",     // 0
   "highpass",    // 1
   "bandpass",    // 2
   "bandreject",  // 3
   NULL
};

inline int _string_to_filtcode(const char *str)
{
   for (int i = 0; _filttype_name[i] != NULL; i++)
      if (strncmp(str, _filttype_name[i], 5) == 0)   // 5 is min. to distinguish
         return i;
   return -1;
}

FiltType BUTTER :: getFiltType(bool trystring)
{
   double index = (double) currentFrame() / nSamps();
   const PField &field = getPField(4);

   // must try int first, since a valid code cast to char * will crash strncmp
   int code = field.intValue(index) - 1;  // NB: user codes are 1-based
   if ((code < LowPass || code > BandReject) && trystring) {
      const char *str = field.stringValue(index);
      code = _string_to_filtcode(str);   // -1 if no match
      if (code != -1)
         filttype_was_string = true;
   }

   FiltType filttype;
   switch (code) {
      case 0: filttype = LowPass;      break;
      case 1: filttype = HighPass;     break;
      case 2: filttype = BandPass;     break;
      case 3: filttype = BandReject;   break;
      default: filttype = FiltInvalid; break;
   }
   return filttype;
}


int BUTTER :: init(double p[], int n_args)
{
   nargs = n_args;
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   amp = p[3];
   nfilts = n_args > 5 ? (int) p[5] : 1;
   do_balance = n_args > 6 ? (bool) p[6] : true;
   inchan = n_args > 7 ? (int) p[7] : 0;            // default is chan 0

   if (rtsetinput(inskip, this) != 0)
      return DONT_SCHEDULE;
   if (inchan >= inputChannels())
      return die("BUTTER", "You asked for channel %d of a %d-channel file.",
                                                      inchan, inputChannels());
   const float ringdur = 0.1;
   if (rtsetoutput(outskip, dur + ringdur, this) == -1)
      return DONT_SCHEDULE;
   insamps = (int) (dur * SR + 0.5);

   if (nfilts < 1 || nfilts > MAXFILTS)
      return die("BUTTER",
                 "Steepness (p5) must be an integer between 1 and %d.",
                 MAXFILTS);

   type = getFiltType(true);
   if (type == FiltInvalid)
      return die("BUTTER", "Type must be \"lowpass\", \"highpass\", "
                           "\"bandpass\", or \"bandreject\".");

   for (int i = 0; i < nfilts; i++)
      filt[i] = new Butter(SR);

   balancer = new Balance(SR);
   balancer->setWindowSize(BALANCE_WINDOW_SIZE);

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(SR, dur, lenamp, amptabs);
   }

   if (n_args < 11) {      // no p10 filter freq PField, must use gen table
      cfarray = floc(2);
      if (cfarray == NULL)
         return die("BUTTER", "Either use the filter frequency pfield (p10) "
                    "or make an old-style gen function in slot 2.");
      int len = fsize(2);
      tableset(SR, dur, len, cftabs);
   }

   if (type == BandPass || type == BandReject) {
      if (n_args < 12) {   // no p11 filter bandwidth PField, must use gen table
         bwarray = floc(3);
         if (bwarray == NULL)
            return die("BUTTER", "Either use the filter bandwidth pfield (p11) "
                       "or make an old-style gen function in slot 3.");
         int len = fsize(3);
         tableset(SR, dur, len, bwtabs);
      }
   }

   skip = (int) (SR / (float) resetval);

   return nSamps();
}


void BUTTER :: doupdate()
{
   double p[12];
   update(p, 12, kType | kPan | kBypass | kFreq | kBandwidth);

   amp = update(3, insamps);
   if (amparray)
      amp *= tablei(currentFrame(), amparray, amptabs);

   // If init type spec was string, we don't allow type updates.
   if (!filttype_was_string) {
      FiltType newtype = getFiltType(false);
      if (newtype == FiltInvalid)
         newtype = LowPass;
      if (newtype != type) {
         type = newtype;
         cf = -FLT_MAX;             // force coefficient changes below
      }
   }

   pctleft = nargs > 8 ? p[8] : 0.5;                // default is center
   bypass = nargs > 9 ? (bool) p[9] : false;        // default is no

   bool setfilt = false;

   float newcf;
   if (nargs > 10)
      newcf = p[10];
   else
      newcf = tablei(currentFrame(), cfarray, cftabs);
   if (newcf < 1.0)
      newcf = 1.0;
   else if (newcf > SR * 0.5)
      newcf = SR * 0.5;
   if (newcf != cf) {
      cf = newcf;
      setfilt = true;
   }

   if (type == BandPass || type == BandReject) {
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
      if (newbw != bw) {
         bw = newbw;
         setfilt = true;
      }
   }

   if (setfilt) {
      if (type == LowPass)
         for (int j = 0; j < nfilts; j++)
            filt[j]->setLowPass(cf);
      else if (type == HighPass)
         for (int j = 0; j < nfilts; j++)
            filt[j]->setHighPass(cf);
      else if (type == BandPass)
         for (int j = 0; j < nfilts; j++)
            filt[j]->setBandPass(cf, bw);
      else // type == BandReject
         for (int j = 0; j < nfilts; j++)
            filt[j]->setBandReject(cf, bw);
   }
}


int BUTTER :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int BUTTER :: run()
{
   int samps = framesToRun() * inputChannels();

   if (currentFrame() < insamps)
      rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         doupdate();
         branch = skip;
      }

      float insig;
      if (currentFrame() < insamps)
         insig = in[i + inchan] * amp;
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

      if (outputChannels() == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeBUTTER()
{
   BUTTER *inst;

   inst = new BUTTER();
   inst->set_bus_config("BUTTER");

   return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
   RT_INTRO("BUTTER", makeBUTTER);
}
#endif
