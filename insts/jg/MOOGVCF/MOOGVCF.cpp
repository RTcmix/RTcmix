/* MOOGVCF - a 24dB/octave resonant lowpass filter

   This is based on the design by Stilson and Smith (CCRMA), as modified
   by Paul Kellett <paul.kellett@maxim.abel.co.uk> (described in the source
   code archives of the Music DSP site -- musicdsp.org).

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier *
   p4 = input channel [optional, default is 0]
   p5 = pan (in percent-to-left form: 0-1) [optional, default is .5]
   p6 = bypass filter (0: no, 1: yes) [optional, default is 0]
   p7 = filter cutoff frequency (Hz) [optional; if missing, must use gen 2] **
   p8 = filter resonance (0-1) [optional; if missing, must use gen 3] ***

   p3 (amplitude), p5 (pan), p6 (bypass), p7 (cutoff) and p8 (resonance) can
   receive dynamic updates from a table or real-time control source.

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p3 (amplitude), even if the latter is dynamic.

   ** If p7 is missing, you must use an old-style gen table 2 for the
   filter cutoff frequency curve.

   *** If p8 is missing, you must use an old-style gen table 3 for the
   filter resonance curve.


   John Gibson <johgibso at indiana dot edu>, 22 May 2002; rev for v4, 7/24/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <Instrument.h>
#include "MOOGVCF.h"
#include <rt.h>
#include <rtdefs.h>
#include <float.h>   // for FLT_MAX


MOOGVCF :: MOOGVCF() : Instrument()
{
   in = NULL;
   cfarray = NULL;
   resarray = NULL;
   branch = 0;
   cf = -FLT_MAX;
   res = -FLT_MAX;
   b0 = b1 = b2 = b3 = b4 = 0.0;
}


MOOGVCF :: ~MOOGVCF()
{
   delete [] in;
}


int MOOGVCF :: init(double p[], int n_args)
{
   nargs = n_args;
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   inchan = n_args > 4 ? (int) p[4] : 0;             // default is chan 0

   float ringdown = 0.2;      // just a guess
   if (rtsetoutput(outskip, dur + ringdown, this) == -1)
      return DONT_SCHEDULE;
   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;

   if (inchan >= inputChannels())
      return die("MOOGVCF", "You asked for channel %d of a %d-channel file.",
                                                   inchan, inputChannels());

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(SR, dur, lenamp, amptabs);
   }

   if (n_args < 8) {       // no p7 cutoff freq PField, must use gen table
      cfarray = floc(2);
      if (cfarray == NULL)
         return die("MOOGVCF", "Either use the cutoff frequency pfield (p7) "
                    "or make an old-style gen function in slot 2.");
      int len = fsize(2);
      tableset(SR, dur, len, cftabs);
   }

   if (n_args < 9) {       // no p9 resonance PField, must use gen table
      resarray = floc(3);
      if (resarray == NULL)
         return die("MOOGVCF", "Either use the resonance pfield (p8) "
                    "or make an old-style gen function in slot 3.");
      int len = fsize(3);
      tableset(SR, dur, len, restabs);
   }

   skip = (int) (SR / (float) resetval);

   return nSamps();
}


void MOOGVCF :: doupdate()
{
   double p[9];
   update(p, 9, kAmp | kPan | kBypass | kCutoff | kResonance);

   amp = p[3];
   if (amparray)
      amp *= tablei(currentFrame(), amparray, amptabs);

   pctleft = nargs > 5 ? p[5] : 0.5;                  // default is .5
   bypass = nargs > 6 ? (bool) p[6] : false;          // default is no

   float newcf;
   if (nargs > 7)
      newcf = p[7];
   else if (cfarray)
      newcf = tablei(currentFrame(), cfarray, cftabs);
   else
      newcf = 1000.0;

   float newres;
   if (nargs > 8)
      newres = p[8];
   else if (resarray)
      newres = tablei(currentFrame(), resarray, restabs);
   else
      newres = 0.5;

   if (newcf != cf || newres != res) {
      cf = newcf;
      res = newres;
      make_coefficients();
   }
}


int MOOGVCF :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int MOOGVCF :: run()
{
   const int samps = framesToRun() * inputChannels();
   rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         doupdate();
         branch = skip;
      }

      float insig = in[i + inchan] * amp;

      float out[2];

      if (bypass)
         out[0] = insig;
      else {
         // Bring (normally) into range [-1, 1]; filter code assumes this.
         insig *= (1.0 / 32768.0);

         insig -= q * b4;           // feedback

         // four cascaded onepole filters (bilinear transform)
         float t1, t2;
         t1 = b1;  b1 = (insig + b0) * p - b1 * f;
         t2 = b2;  b2 = (b1 + t1)    * p - b2 * f;
         t1 = b3;  b3 = (b2 + t2)    * p - b3 * f;
                   b4 = (b3 + t1)    * p - b4 * f;
         b4 = b4 - b4 * b4 * b4 * 0.166667;           // clipping
         b0 = insig;

#if defined(HIGHPASS)      // but only 6dB/oct
         out[0] = insig - b4;
#elif defined(BANDPASS)    // 6dB/oct
         out[0] = 3.0 * (b3 - b4);
#else
         out[0] = b4;
#endif
         out[0] *= 32768.0;
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


Instrument *makeMOOGVCF()
{
   MOOGVCF *inst;

   inst = new MOOGVCF();
   inst->set_bus_config("MOOGVCF");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("MOOGVCF", makeMOOGVCF);
}
#endif
