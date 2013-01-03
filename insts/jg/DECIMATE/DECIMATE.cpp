/* DECIMATE - reduce number of bits used to represent sound

   NOTE: Pfield order has changed since v3.8!  (p4 is new)

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = pre-amp multiplier (before decimation)
   p4 = post-amp multiplier (after decimation)
   p5 = number of bits to use (1 to 16)
   p6 = low-pass filter cutoff frequency (or 0 to bypass)
         [optional, default is 0]
   p7 = input channel [optional, default is 0]
   p8 = percent of signal to left output channel [optional, default is .5]

   p3 (pre-amp), p4 (post-amp), p5 (bits), p6 (cutoff) and p8 (pan) can
   receive dynamic updates from a table or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p4 post-amp multiplier, even if the latter is dynamic.

   JGG <johgibso at indiana dot edu>, 3 Jan 2002, rev for v4, 7/11/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include "DECIMATE.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG
//#define DEBUG2


DECIMATE :: DECIMATE() : Instrument()
{
   in = NULL;
   lpfilt = NULL;
   branch = 0;
   warn_bits = true;
   warn_cutoff = true;
}


DECIMATE :: ~DECIMATE()
{
   delete [] in;
   delete lpfilt;
}


static void warnold()
{
   rtcmix_warn("DECIMATE", "Pfield order has changed since v3.8!  p4 is new.");
}


int DECIMATE :: init(double p[], int n_args)
{
   nargs = n_args;
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   preamp = p[3];
   postamp = p[4];
   bits = (int) p[5];
   cutoff = n_args > 6 ? p[6] : 0.0;                 // default is bypass
   inchan = n_args > 7 ? (int) p[7] : 0;             // default is chan 0

   // Prior to v4, pre-amp was given by p3 amp and post-amp was given by
   // optional setline.  The latter now must be a pfield, and for clarity,
   // it's been placed just after p3, in p4.  We try to warn users in the 
   // right circumstances.  Fortunately, there are two situations that help
   // us to identify scores with the old argument order.
   //
   //    1. Fewer than 6 pfields means it's an old score with none of
   //       the optional pfields.
   //
   //    2. Old scores will almost certainly have an initial out-of-range
   //       value for bits (p5).  Formerly p5 was low-pass cutoff frequency,
   //       with 0 meaning filter bypass.  0 is invalid for bits, and the
   //       valid bit values are two low for a useable low-pass filter.

   if (n_args < 6) {
      warnold();
      return die("DECIMATE", "Must have at least 6 pfields.");
   }

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;

   if (inchan >= inputChannels())
      return die("DECIMATE", "You asked for channel %d of a %d-channel file.",
                                                      inchan, inputChannels());

   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;

   if (bits > 16 || bits < 1) {
      warnold();
      return die("DECIMATE", "Bits must be between 1 and 16.");
   }
   changebits(bits);

   if (preamp < 0.0)
      return die("DECIMATE", "pre-amp must be zero or greater.");
   if (postamp < 0.0)
      return die("DECIMATE", "post-amp must be zero or greater.");

   amparray = floc(1);
   if (amparray) {
      int len = fsize(1);
      tableset(SR, dur, len, amptabs);
   }

   nyquist = SR * 0.5;
   if (cutoff < 0.0 || cutoff > nyquist)
      return die("DECIMATE",
                 "Cutoff frequency must be between 0 and %g.", nyquist);

   usefilt = (cutoff > 0.0);
   if (usefilt) {
      lpfilt = new Butter(SR);
      lpfilt->setLowPass(cutoff);
   }
   else
      rtcmix_advise("DECIMATE", "Disabling low-pass filter.");

   skip = (int) (SR / (float) resetval);

   return nSamps();
}


int DECIMATE :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


inline void DECIMATE :: changebits(int nbits)
{
   if (nbits > 16 || nbits < 1) {
      if (warn_bits) {
         rtcmix_warn("DECIMATE", "Bits must be between 1 and 16.");
         warn_bits = false;
      }
      nbits = (nbits > 16) ? 16 : 1;
   }

   mask = (int32_t) ( ((int) pow(2.0, (double) nbits) - 1) << (16 - nbits) );
   mask |= 0xFFFF0000;
   if (nbits == 16)
      bias = 0;
   else {
      int power = (16 - nbits) - 1;
      bias = (int) pow(2.0, (double) power);
   }
#ifdef DEBUG
   printf("mask = 0x%X, bias = %d\n", mask, bias);
#endif
}


int DECIMATE :: run()
{
   int samps = framesToRun() * inputChannels();
   rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         double p[9];
         update(p, 9);
         preamp = p[3];
         postamp = p[4];
         if (amparray)        // backward-compat
            postamp *= tablei(cursamp, amparray, amptabs);
         int newbits = (int) p[5];
         if (newbits != bits) {
            bits = newbits;
            changebits(bits);
         }
         if (usefilt && p[6] != cutoff) {
            cutoff = p[6];
            if (cutoff < 0.0 || cutoff > nyquist) {
               if (warn_cutoff) {
                  rtcmix_warn("DECIMATE",
                       "Cutoff frequency must be between 0 and %g.", nyquist);
                  warn_cutoff = false;
               }
               cutoff = nyquist * 0.5;
            }
            lpfilt->setLowPass(cutoff);
         }
         pctleft = nargs > 8 ? p[8] : 0.5;                // default is .5
         branch = skip;
      }

      float out[2];
      float sig = in[i + inchan] * preamp;
      int32_t isig = (int32_t) sig;
      out[0] = (float) ((isig & mask) + bias);
#ifdef DEBUG2
      printf("%f -> %d -> %f\n", sig, isig, out[0]);
#endif
      if (usefilt)
         out[0] = lpfilt->tick(out[0]);

      out[0] *= postamp;

      if (outputChannels() == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeDECIMATE()
{
   DECIMATE *inst;

   inst = new DECIMATE();
   inst->set_bus_config("DECIMATE");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("DECIMATE", makeDECIMATE);
}
#endif

