/* MULTEQ - multi-band equalizer instrument

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = master bypass (0: no, 1: yes) 

   Followed by one or more (up to 8) EQ band descriptions, given by
   quintuplets of
        EQ type ("lowpass", "highpass", "lowshelf", "highshelf", "peaknotch",
                 "bandpass") *
        filter frequency (Hz)
        filter Q (c. 0.5-10)
        filter gain (cut or boost, in dB -- for shelf and peak/notch only)
        bypass (0: no, 1: yes)

   So the settings for the first band would occupy pfields 5-9, the second,
   pfields 10-14, and so on.

   p3 (amplitude), p4 (bypass) as well as the freq, Q, gain and bypass pfields
   for individual bands can receive dynamic updates from a table or real-time
   control source.

   The EQ types can be updated only when using numeric codes (0: lowpass,
   1: highpass, 2: lowshelf, 3: highshelf, 4: peaknotch, 5: bandpass).  If
   you give the string version, you can't change types during a note.

   The number of input channels must equal the number of output channels.
   There can be as many as 8 channels.

   John Gibson <johgibso at indiana dot edu>, 26 Sep 2004; derived from EQ.

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
#include "MULTEQ.h"
#include <rt.h>
#include <rtdefs.h>
#include <float.h>   // for FLT_MIN

#define FIRST_BAND_PF 5
#define BAND_PFS      5


EQBand :: EQBand(float srate, OeqType type, float freq, float Q, float gain,
      bool bypass)
   : _type(type), _freq(freq), _Q(Q), _gain(gain), _bypass(bypass)
{
   _eq = new Oequalizer(srate, _type);
   if (_eq)
      _eq->setparams(_freq, _Q, _gain);
}


EQBand :: ~EQBand()
{
   delete _eq;
}


MULTEQ :: MULTEQ() : Instrument()
{
   in = NULL;
   branch = 0;
   for (int i = 0; i < MAXBAND * MAXCHAN; i++)
      eq[i] = NULL;
}


MULTEQ :: ~MULTEQ()
{
   delete [] in;
   for (int i = 0; i < MAXBAND * MAXCHAN; i++)
      delete eq[i];
}


static const char *_eqtype_name[] = {
   "lowpass",     // 0
   "highpass",    // 1
   "lowshelf",    // 2
   "highshelf",   // 3
   "peaknotch",   // 4
   "bandpass",    // 5
   NULL
};

inline int _string_to_eqcode(const char *str)
{
   for (int i = 0; _eqtype_name[i] != NULL; i++)
      if (strncmp(str, _eqtype_name[i], 5) == 0)   // 5 is min. to distinguish
         return i;
   return -1;
}

OeqType MULTEQ :: getEQType(bool trystring, int pfindex)
{
   double index = (double) currentFrame() / nSamps();
   const PField &field = getPField(pfindex);

   // must try int first, since a valid code cast to char * will crash strncmp
   int code = field.intValue(index);
   if (trystring && (code < 0 || code > 4)) {
      const char *str = field.stringValue(index);
      code = _string_to_eqcode(str);   // -1 if no match
   }

   OeqType eqtype;
   switch (code) {
      case 0: eqtype = OeqLowPass;   break;
      case 1: eqtype = OeqHighPass;  break;
      case 2: eqtype = OeqLowShelf;  break;
      case 3: eqtype = OeqHighShelf; break;
      case 4: eqtype = OeqPeaking;   break;
      case 5: eqtype = OeqBandPassCSG; break;
      default: eqtype = OeqInvalid;  break;
   }
   return eqtype;
}


int MULTEQ :: init(double p[], int n_args)
{
   nargs = n_args;
   const float ringdur = 0.1;
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   insamps = (int) (dur * SR + 0.5);

   if (rtsetoutput(outskip, dur + ringdur, this) == -1)
      return DONT_SCHEDULE;

   if (inputChannels() > MAXCHAN)
      return die("MULTEQ",
               "Input and output must have no more than %d channels.", MAXCHAN);
   if (outputChannels() != inputChannels())
      return die("MULTEQ", "Input and output must have same number of "
                           "channels, no more than %d.", MAXCHAN);

   if ((nargs - FIRST_BAND_PF) % BAND_PFS)
      return die("MULTEQ",
                 "For each band, need type, freq, Q, gain and bypass.");

   numbands = 0;
   int band = 0;
   for (int i = FIRST_BAND_PF; i < nargs; i += BAND_PFS, band += MAXCHAN) {
      if (numbands == MAXBAND) {
         rtcmix_warn("MULTEQ", "You can only have %d EQ bands.", MAXBAND);
         break;
      }

      OeqType type = getEQType(true, i);
      if (type == OeqInvalid)
         return die("MULTEQ", "Invalid EQ type string or code.");
      float freq = p[i + 1];
      float Q = p[i + 2];
      float gain = p[i + 3];
      bool bypass = (bool) p[i + 4];

      for (int c = 0; c < inputChannels(); c++) {
         eq[band + c] = new EQBand(SR, type, freq, Q, gain, bypass);
         if (eq[band + c] == NULL)
            return die("MULTEQ", "Can't allocate EQ band.");
      }

      numbands++;
   }

   skip = (int) (SR / (float) resetval);

   return nSamps();
}


void MULTEQ :: doupdate()
{
   double p[nargs];
   update(p, nargs);

   amp = p[3];
   bypass = (bool) p[4];

   int band = 0;
   for (int i = FIRST_BAND_PF; i < nargs; i += BAND_PFS, band += MAXCHAN) {
      OeqType type = getEQType(false, i);
      float freq = p[i + 1];
      if (freq < 0.0)
         freq = 0.0;
      else if (freq > SR * 0.5)
         freq = SR * 0.5;
      float Q = p[i + 2];
      if (Q <= 0.0)
         Q = FLT_MIN;
      float gain = p[i + 3];
      bool bypass = (bool) p[i + 4];

      for (int c = 0; c < inputChannels(); c++)
         eq[band + c]->setparams(type, freq, Q, gain, bypass);
   }
}


int MULTEQ :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int MULTEQ :: run()
{
   const int samps = framesToRun() * inputChannels();
   if (currentFrame() < insamps)
      rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         doupdate();
         branch = skip;
      }

      float insig[MAXCHAN];
      if (currentFrame() < insamps) {
         for (int c = 0; c < inputChannels(); c++)
            insig[c] = in[i + c];
      }
      else {
         for (int c = 0; c < inputChannels(); c++)
            insig[c] = 0.0;
      }

      float out[MAXCHAN];
      if (bypass) {
         for (int c = 0; c < inputChannels(); c++)
            out[c] = insig[c] * amp;
      }
      else {
         for (int c = 0; c < inputChannels(); c++) {
            for (int b = 0; b < numbands; b++) {
               int index = (b * MAXCHAN) + c;
               insig[c] = eq[index]->next(insig[c]);
            }
            out[c] = insig[c] * amp;
         }
      }

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeMULTEQ()
{
   MULTEQ *inst;

   inst = new MULTEQ();
   inst->set_bus_config("MULTEQ");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("MULTEQ", makeMULTEQ);
}
#endif
