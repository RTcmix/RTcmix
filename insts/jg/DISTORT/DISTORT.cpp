/* DISTORT - time-varying Butterworth filters

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = type of distortion (1: soft clip, 2: tube)
        [NOTE: 2 doesn't work correctly yet!]
   p5 = gain (before distortion)
   p6 = cutoff freq for low-pass filter (in cps)  (0 to disable filter)
        (The filter comes after the distortion in the signal chain.)
   p7 = input channel [optional, default is 0]
   p8 = percent to left channel [optional, default is .5]
   p9 = bypass all processing (0: no, 1: yes) [optional, default is 0]

   Function table 1 is the amplitude envelope.

   John Gibson (johgibso at indiana dot edu), 8/12/03.
   Distortion algorithms taken from STRUM, by Charlie Sullivan.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "DISTORT.h"
#include <rt.h>
#include <rtdefs.h>


DISTORT :: DISTORT() : Instrument()
{
   in = NULL;
   filt = NULL;      // might not create
   amptable = NULL;  // might not create
   branch = 0;
}


DISTORT :: ~DISTORT()
{
   delete [] in;
   delete filt;
}


int DISTORT :: init(double p[], int n_args)
{
   float outskip, inskip, dur, cf;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   type = (DistortType) p[4];
   gain = p[5];
   cf = n_args > 6 ? p[6] : 0.0;                   /* filter disabled */
   inchan = n_args > 7 ? (int) p[7] : 0;           /* default is chan 0 */
   pctleft = n_args > 8 ? p[8] : 0.5;              /* default is center */
   bypass = n_args > 9 ? (int) p[9] : 0;           /* default is no */

   if (rtsetinput(inskip, this) != 0)
      return DONT_SCHEDULE;
   nsamps = rtsetoutput(outskip, dur, this);

   if (inchan >= inputChannels())
      return die("DISTORT", "You asked for channel %d of a %d-channel file.",
                                                      inchan, inputChannels());
   if (type != SoftClip && type != Tube)
      return die("DISTORT",
                 "Distortion type must be 1 (soft clip) or 2 (tube).");

   if (cf > 0.0) {
      filt = new Butter();
      filt->setLowPass(cf);
   }

   float *function = floc(1);
   if (function) {
      int len = fsize(1);
      amptable = new TableL(dur, function, len);
   }
   else {
      aamp = amp;
      advise("DISTORT", "Setting phrase curve to all 1's.");
   }

   skip = (int)(SR / (float)resetval);

   return nSamps();
}


/* Process a signal in range [-1, 1] with various types of distortion. */
float DISTORT :: distort(float sig, float g)
{
   if (type == SoftClip) {
      /* Soft clipping: dist = x - 1/3 * x^3  */
      sig *= g;
      if (sig > 1.0)
         return .66666667;
      else if (sig > -1.0)
         return sig - (.33333333 * sig * sig * sig);
      else
         return -.66666667;
   }
   else if (type == Tube) {
// FIXME: this doesn't work yet...
      sig *= g;
      /* Tube-ish distortion: dist = (x +.5)^2 -.25  */
      /* Charlie says: 'this does not work with a feedback guitar' */
      float out = (sig + 0.5) * (sig + 0.5) - 0.25;
      return out / (g * 2.0);
   }

/* other stuff...*/
/*
1. Variable-hardness clipping function

References: Posted by Laurent de Soras
[See http://musicdsp.org/files/laurent.gif for an animated graph.]

Notes:
k >= 1 is the "clipping hardness". 1 gives a smooth clipping, and a high value
gives hardclipping.  Don't set k too high, because the formula use the pow()
function, which use exp() and would overflow easily. 100 seems to be a
reasonable value for "hardclipping"

Code:
f(x) = sign(x) * pow(atan(pow(abs(x), k)), (1 / k));
[surely 'sign' should be 'sin' -JGG]


2. WaveShaper

Type: waveshaper
References: Posted by Bram de Jong

Notes:
where x (in [-1..1] will be distorted and a is a distortion parameter that goes
from 1 to infinity.  The equation is valid for positive and negativ values.  If
a is 1, it results in a slight distortion and with bigger a's the signal get's
more funky.

A good thing about the shaper is that feeding it with bigger-than-one values,
doesn't create strange fx. The maximum this function will reach is 1.2 for a=1.

Code:
f(x,a) = x * (abs(x) + a) / (x^2 + (a - 1) * abs(x) + 1)
*/


   return 0.0;
}


int DISTORT :: run()
{
   float out[2];

   if (in == NULL)                  // first time, so allocate it
      in = new float [RTBUFSAMPS * inputchans];

   const int insamps = framesToRun() * inputChannels();
   rtgetin(in, this, insamps);

   for (int i = 0; i < insamps; i += inputChannels()) {
      if (--branch < 0) {
         if (amptable)
            aamp = amptable->tick(currentFrame(), amp);
         branch = skip;
      }
      float sig = in[i + inchan];
      if (!bypass) {
         sig *= (1.0 / 32768.0);    // convert range
         sig = distort(sig, gain);
         sig *= 32768.0;
         if (filt)
            sig = filt->tick(sig);
      }
      sig *= aamp;
      if (outputchans == 2) {
         out[0] = sig * pctleft;
         out[1] = sig * (1.0 - pctleft);
      }
      else
         out[0] = sig;

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeDISTORT()
{
   DISTORT *inst;

   inst = new DISTORT();
   inst->set_bus_config("DISTORT");

   return inst;
}

void
rtprofile()
{
   RT_INTRO("DISTORT", makeDISTORT);
}
