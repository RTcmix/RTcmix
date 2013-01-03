/* EQ - equalizer instrument (peak/notch, shelving and high/low pass types)

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier *
   p4 = EQ type ("lowpass", "highpass", "lowshelf", "highshelf", "peaknotch") **
   p5 = input channel [optional, default is 0]
   p6 = pan (in percent-to-left form: 0-1) [optional, default is .5]
   p7 = bypass filter (0: no, 1: yes) [optional, default is 0]
   p8 = filter frequency (Hz) [optional; if missing, must use gen 2] ***
   p9 = filter Q (values from 0.5 to 10.0, roughly) [optional; if missing,
        must use gen 3] ****
   p10 = filter gain (dB) [shelf and peak/notch only; if missing, must
         use gen 4] *****

   p3 (amplitude), p4 (type), p6 (pan), p7 (bypass), p8 (freq), p9 (Q)
   and p10 (gain) can receive dynamic updates from a table or real-time
   control source.  p4 (type) can be updated only when using numeric codes. **

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p3 (amplitude), even if the latter is dynamic.

   ** You can also give numeric codes for the EQ type (0: lowpass, 1: highpass,
   2: lowshelf, 3: highshelf, 4: peaknotch).  These can be changed during a
   note.  If you give the string version, you can't change types during a note.

   *** If p8 is missing, you must use an old-style gen table 2 for the
   filter frequency curve.

   **** If p9 is missing, you must use an old-style gen table 3 for the
   filter Q curve.

   ***** If p10 is missing, you must use an old-style gen table 4 for the
   filter gain curve.


   John Gibson <johgibso at indiana dot edu>, 7 Dec 2003; rev for v4, 7/23/04

   Based on formulas by Robert Bristow-Johnson ("Audio-EQ-Cookbook") and code
   by Tom St Denis <tomstdenis.home.dhs.org> (see musicdsp.org)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ugens.h>
#include <math.h>
#include <Instrument.h>
#include <PField.h>
#include "EQ.h"
#include <rt.h>
#include <rtdefs.h>
#include <float.h>   // for FLT_MIN, FLT_MAX


EQ :: EQ() : Instrument()
{
   in = NULL;
   amp_table = NULL;
   freq_table = NULL;
   q_table = NULL;
   gain_table = NULL;
   branch = 0;
   freq = Q = gain = -FLT_MAX;
   eqtype_was_string = false;
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


static const char *_eqtype_name[] = {
   "lowpass",     // 0
   "highpass",    // 1
   "lowshelf",    // 2
   "highshelf",   // 3
   "peaknotch",   // 4
   NULL
};

inline int _string_to_eqcode(const char *str)
{
   for (int i = 0; _eqtype_name[i] != NULL; i++)
      if (strncmp(str, _eqtype_name[i], 5) == 0)   // 5 is min. to distinguish
         return i;
   return -1;
}

EQType EQ :: getEQType(bool trystring)
{
   double index = (double) currentFrame() / nSamps();
   const PField &field = getPField(4);

   // must try int first, since a valid code cast to char * will crash strncmp
   int code = field.intValue(index);
   if ((code < 0 || code > 4) && trystring) {
      const char *str = field.stringValue(index);
      code = _string_to_eqcode(str);   // -1 if no match
      if (code != -1)
         eqtype_was_string = true;
   }

   EQType eqtype;
   switch (code) {
      case 0: eqtype = EQLowPass;   break;
      case 1: eqtype = EQHighPass;  break;
      case 2: eqtype = EQLowShelf;  break;
      case 3: eqtype = EQHighShelf; break;
      case 4: eqtype = EQPeaking;   break;
      default: eqtype = EQInvalid;  break;
   }
   return eqtype;
}


int EQ :: init(double p[], int n_args)
{
   nargs = n_args;
   const float ringdur = 0.1;
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   inchan = n_args > 5 ? (int) p[5] : 0;           // default is chan 0

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   insamps = (int) (dur * SR + 0.5);
   if (inchan >= inputChannels())
      return die("EQ", "You asked for channel %d of a %d-channel file.",
                                                   inchan, inputChannels());
   if (rtsetoutput(outskip, dur + ringdur, this) == -1)
      return DONT_SCHEDULE;
   if (outputChannels() > 2)
      return die("EQ", "Output must be mono or stereo.");

   eqtype = getEQType(true);
   if (eqtype == EQInvalid)
      return die("EQ", "Type must be \"lowpass\", \"highpass\", \"lowshelf\", "
                       "\"highshelf\" or \"peaknotch\".");

   double *function = floc(1);
   if (function) {
      int len = fsize(1);
      amp_table = new TableL(SR, dur, function, len);
   }

   if (n_args < 9) {       // no p8 filter freq PField, must use gen table
      function = floc(2);
      if (function == NULL)
         return die("EQ", "Either use the EQ frequency pfield (p8) or make "
                    "an old-style gen function in slot 2.");
      int len = fsize(2);
      freq_table = new TableL(SR, dur, function, len);
   }

   if (n_args < 10) {      // no p9 filter Q PField, must use gen table
      function = floc(3);
      if (function == NULL)
         return die("EQ", "Either use the EQ Q pfield (p9) or make "
                    "an old-style gen function in slot 3.");
      int len = fsize(3);
      q_table = new TableL(SR, dur, function, len);
   }

   if (eqtype != EQLowPass && eqtype != EQHighPass) {
      if (n_args < 11) {      // no p10 filter gain PField, must use gen table
         function = floc(4);
         if (function == NULL)
            return die("EQ", "Either use the EQ gain pfield (p10) or make "
                       "an old-style gen function in slot 4.");
         int len = fsize(4);
         gain_table = new TableL(SR, dur, function, len);
      }
   }

   eq = new Equalizer(SR, eqtype);

   skip = (int) (SR / (float) resetval);

   return nSamps();
}


void EQ :: doupdate()
{
   double p[11];
   update(p, 11, kAmp | kType | kPan | kBypass | kFreq | kQ | kGain);

   amp = p[3];
   if (amp_table)
      amp *= amp_table->tick(currentFrame(), 1.0);

   if (!eqtype_was_string) {
      EQType type = getEQType(false);
      if (type == EQInvalid)
         type = EQLowPass;
      if (type != eqtype) {
         eqtype = type;
         delete eq;
         eq = new Equalizer(SR, eqtype);
         freq = -FLT_MAX;              // force setCoeffs call below
      }
   }

   pctleft = nargs > 6 ? p[6] : 0.5;            // default is .5
   bypass = nargs > 7 ? (bool) p[7] : false;    // default is no

   float newfreq;
   if (nargs > 8)
      newfreq = p[8];
   else
      newfreq = freq_table->tick(currentFrame(), 1.0);
   if (newfreq < 0.0)
      newfreq = 0.0;
   else if (newfreq > SR * 0.5)
      newfreq = SR * 0.5;

   float newQ;
   if (nargs > 9)
      newQ = p[9];
   else
      newQ = q_table->tick(currentFrame(), 1.0);
   if (newQ <= 0.0)
      newQ = FLT_MIN;

   float newgain;
   if (nargs > 10)
      newgain = p[10];
   else if (gain_table)
      newgain = gain_table->tick(currentFrame(), 1.0);
   else
      newgain = 0.0;

   if (newfreq != freq || newQ != Q || newgain != gain) {
      freq = newfreq;
      Q = newQ;
      gain = newgain;
      eq->setCoeffs(freq, Q, gain);
   }
}


int EQ :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int EQ :: run()
{
   const int samps = framesToRun() * inputChannels();
   if (currentFrame() < insamps)
      rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         doupdate();
         branch = skip;
      }

      float insig = 0.0;
      if (currentFrame() < insamps)
         insig = in[i + inchan];

      float outsig;
      if (bypass)
         outsig = insig;
      else
         outsig = eq->tick(insig);

      float out[2];
      out[0] = outsig * amp;

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

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("EQ", makeEQ);
}
#endif
