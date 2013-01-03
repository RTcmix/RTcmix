/* MULTIWAVE - additive synthesis instrument

      p0 = output start time
      p1 = duration
      p2 = overall amplitude (0-32767, as in WAVETABLE)
      p3 = wavetable (use maketable("wave", ...) for this)

      Then any number of quadruplets defining the frequency, amplitude,
      phase and panning of individual partials.

      p4 = frequency (Hz)
      p5 = amplitude (0-1)
      p6 = initial phase (0-360 degrees, not updateable)
      p7 = pan (pctleft: 0-1)

   p2 (amplitude), p4 (freq), p5 (partial amp), p7 (pan), and the same
   parameters for additional partials, can receive updates from a table
   or real-time control source.

   John Gibson, 3/9/05
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Ougens.h>
#include <Instrument.h>
#include <PField.h>
#include "MULTIWAVE.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG

#define FIRST_FREQ_ARG  4


MULTIWAVE::MULTIWAVE() : Instrument()
{
   branch = 0;
   numpartials = 0;
   oscil = NULL;
   amp = NULL;
   pan = NULL;
}


MULTIWAVE::~MULTIWAVE()
{
   for (int i = 0; i < numpartials; i++)
      delete oscil[i];
   delete [] oscil;
   delete [] amp;
   delete [] pan;
}


int MULTIWAVE::usage()
{
   return die("MULTIWAVE",
              "Usage: MULTIWAVE(start, dur, amp, wavetable,\n"
              "                 freq1, amp1, phase1, pan1\n"
              "                 [, freq2, amp2, phase2, pan2, ...]");
}


int MULTIWAVE::init(double p[], int n_args)
{
   if (n_args < 8)
      return usage();
   float outskip = p[0];
   float dur = p[1];
   nargs = n_args;

   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;
   if (outputChannels() < 1 || outputChannels() > 2)
      return die("MULTIWAVE", "Must have mono or stereo output only.");

   int wavelen;
   double *wavet = (double *) getPFieldTable(3, &wavelen);
   if (wavet == NULL)
      return die("MULTIWAVE", "p3 must be wavetable (use maketable)");

   numpartials = (nargs - FIRST_FREQ_ARG) / 4;
   oscil = new Ooscili * [numpartials];
   amp = new double [numpartials];
   pan = new double [numpartials];

   for (int i = 0; i < numpartials; i++) {
      oscil[i] = new Ooscili(SR, 440.0, wavet, wavelen);
      const int index = FIRST_FREQ_ARG + (4 * i);
      oscil[i]->setphase(p[index + 2] / 360.0);
      amp[i] = 0.0;
      pan[i] = 0.0;
   }

   return nSamps();
}


void MULTIWAVE::doupdate()
{
   double p[nargs];
   update(p, nargs);

   overall_amp = p[2];

   for (int i = 0; i < numpartials; i++) {
      const int index = FIRST_FREQ_ARG + (4 * i);
      oscil[i]->setfreq(p[index]);
      amp[i] = p[index + 1];
      pan[i] = p[index + 3];
   }
}


int MULTIWAVE::run()
{
   const int samps = framesToRun();
   const int chans = outputChannels();
   float out[chans];

   for (int i = 0; i < samps; i++) {
      if (--branch <= 0) {
         doupdate();
         branch = getSkip();
      }

      for (int j = 0; j < chans; j++)
         out[j] = 0.0f;

      for (int j = 0; j < numpartials; j++) {
         float sig = oscil[j]->next() * amp[j];
         if (chans == 1)
            out[0] += sig;
         else {
            out[0] += sig * pan[j];
            out[1] += sig * (1.0 - pan[j]);
         }
      }
      float scale = (1.0 / float(numpartials)) * (overall_amp * chans);
      for (int j = 0; j < chans; j++)
         out[j] *= scale;

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeMULTIWAVE()
{
   MULTIWAVE *inst;

   inst = new MULTIWAVE();
   inst->set_bus_config("MULTIWAVE");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("MULTIWAVE", makeMULTIWAVE);
}
#endif
