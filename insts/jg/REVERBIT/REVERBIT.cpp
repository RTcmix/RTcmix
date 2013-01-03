/* REVERBIT - Lansky's fast reverberator
  
   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier (of input signal)
   p4 = reverb time (must be greater than 0)
   p5 = reverb percent (between 0 and 1 inclusive)
   p6 = right channel delay time (must be greater than 0)
   p7 = cutoff freq for low-pass filter (in cps)  (0 to disable filter)
   p8 = apply DC blocking filter (0: No, 1: Yes) [optional, default is Yes]
        (set to zero for a bit more speed)
   p9 = ring-down duration [optional, default is first reverb time value]

   Input file can be mono or stereo. Output file must be stereo.

   p3 (amplitude), p4 (reverb time), p5 (reverb percent) and p7 (cutoff)
   can receive dynamic updates from a table or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   The amplitude multiplier is applied to the input sound *before*
   it enters the reverberator.

   The point of the ring-down duration parameter is to let you control
   how long the reverb will ring after the input has stopped.  If the
   reverb time is constant, REVERBIT will figure out the correct ring-down
   duration for you.  If the reverb time is dynamic, you must specify a
   ring-down duration if you want to ensure that your sound will not be
   cut off prematurely.

   To quote Lansky, this is meant to "put a gloss on a signal."
   Here's how it works:
     (1) Runs the input signal into a simple Schroeder reverberator,
         scaling the output of that by the reverb percent and flipping
         its phase.
     (2) Puts output of (1) into a delay line, length determined by p6.
     (3) Adds output of (1) to dry signal, and places in left channel.
     (4) Adds output of delay to dry signal, and places in right channel.

   I added the optional low-pass filter to the output of (1) and the
   optional DC blocking filter.

   RT'd and spruced up a bit by John Gibson (jgg9c@virginia.edu), 6/24/99
   rev for v4, 7/11/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "REVERBIT.h"
#include <rt.h>
#include <rtdefs.h>

#define DELAY_FACTOR   2       // Determines length of delay line
#define MIN_DELAY      .10     // Uses extra-large delay line below this
#define RVTSLOP        0.2     // Extra ring-down time

/* The DC-blocking code is from Perry Cook's STK (STK98v2/DCBlock.cpp).
   It works pretty well (and quickly) for all but extreme cases. But I've
   gotten REVERBIT to generate files with a fair amount of DC even *with* this
   simple filter. The Remove DC function in mxv works better. It's the standard
   cmix elliptical filter with 60db atten., stopband at 5 hz, and passband at
   20 hz. But that's overkill for here.
*/

// local functions
static void toneset(double, double, int, double []);
static double tone(double, double []);



REVERBIT::REVERBIT() : Instrument()
{
   in = NULL;
   delarray = rvbarray = NULL;
   branch = 0;
}

REVERBIT::~REVERBIT()
{
   delete [] in;
   delete [] delarray;
   delete [] rvbarray;
}

int REVERBIT::init(double p[], int n_args)
{
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   reverbtime = p[4];
   reverbpct = p[5];
   rtchan_delaytime = p[6];
   cutoff = p[7];
   dcblock = n_args > 8 ? (p[8] != 0.0) : true;      // default is "yes"
   float ringdur = n_args > 9 ? p[9] : reverbtime + RVTSLOP;

   if (outputChannels() != 2)
      return die("REVERBIT", "Output must be stereo.");

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   if (inputChannels() > 2)
      return die("REVERBIT", "Can't have more than 2 input channels.");

   if (rtsetoutput(outskip, dur + ringdur, this) == -1)
      return DONT_SCHEDULE;
   insamps = (int) (dur * SR);

   if (reverbtime <= 0.0)
      return die("REVERBIT", "Reverb time must be greater than 0.");

   if (reverbpct < 0.0 || reverbpct > 1.0)
      return die("REVERBIT",
                 "Reverb percent must be between 0 and 1 inclusive.");

   if (rtchan_delaytime <= 0.0)
      return die("REVERBIT", "Right chan delay time must be greater than 0.");

   if (cutoff < 0.0)
      return die("REVERBIT", "Cutoff frequency should be positive (or zero to "
                                                           "disable filter).");
   usefilt = (cutoff > 0.0);
   if (usefilt)
      toneset(SR, cutoff, 1, tonedata);
   else
      rtcmix_advise("REVERBIT", "Low-pass filter disabled.");

   float maxdeltime = rtchan_delaytime;
   // If delay time is very short, make delay line longer than necessary.
   if (rtchan_delaytime < MIN_DELAY)
      maxdeltime *= DELAY_FACTOR;
   int delsamps = (int) (maxdeltime * SR + 0.5);
   delarray = new float[delsamps];
   delset(SR, delarray, deltabs, maxdeltime);

   // Array dimensions taken from lib/rvbset.c (+ 2 extra for caution).
   int rvbsamps = (int)((0.1583 * SR) + 18 + 2);
   rvbarray = new float[rvbsamps];
   rvbset(SR, reverbtime, 0, rvbarray);

   amparray = floc(1);
   if (amparray) {
      int amplen = fsize(1);
      tableset(SR, dur, amplen, amptabs);
   }

   prev_in[0] = prev_out[0] = 0.0;         // for DC-blocker
   prev_in[1] = prev_out[1] = 0.0;

   return nSamps();
}


int REVERBIT::configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


void REVERBIT::doupdate()
{
   double p[8];
   update(p, 8, kRvbTime | kRvbPct | kCutoffFreq);

   amp = update(3, insamps);  // env spans only input dur
   if (amparray)
      amp *= tablei(currentFrame(), amparray, amptabs);

   if (p[4] != reverbtime) {
      reverbtime = p[4];
      if (reverbtime <= 0.0)
         reverbtime = 0.0001;
      rvbset(SR, reverbtime, 1, rvbarray);
   }

   reverbpct = p[5];
   if (reverbpct < 0.0)
      reverbpct = 0.0;
   else if (reverbpct > 1.0)
      reverbpct = 1.0;

   if (usefilt && p[7] != cutoff) {
      cutoff = p[7];
      if (cutoff <= 0.0)
         cutoff = 0.01;
      toneset(SR, cutoff, 0, tonedata);
   }
}


int REVERBIT::run()
{
   int samps = framesToRun() * inputChannels();

   if (currentFrame() < insamps)
      rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         doupdate();
         branch = getSkip();
      }

      float insig[2], out[2];

      if (currentFrame() < insamps) {        // still taking input from file
         insig[0] = in[i] * amp;
         insig[1] = (inputChannels() == 2) ? in[i + 1] * amp : insig[0];
      }
      else                                   // in ring-down phase
         insig[0] = insig[1] = 0.0;

      float rvbsig = -reverbpct * reverb(insig[0] + insig[1], rvbarray);

      if (usefilt)
         rvbsig = tone(rvbsig, tonedata);

      delput(rvbsig, delarray, deltabs);
      float delsig = delget(delarray, rtchan_delaytime, deltabs);

      out[0] = insig[0] + rvbsig;
      out[1] = insig[1] + delsig;

      if (dcblock) {
         float tmp_in[2];

         tmp_in[0] = out[0];
         tmp_in[1] = out[1];

         out[0] = tmp_in[0] - prev_in[0] + (0.99 * prev_out[0]);
         prev_in[0] = tmp_in[0];
         prev_out[0] = out[0];

         out[1] = tmp_in[1] - prev_in[1] + (0.99 * prev_out[1]);
         prev_in[1] = tmp_in[1];
         prev_out[1] = out[1];
      }

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


/* -------------------------------------------------------------------------- */
/* This filter nabbed from Doug Scott's place program */

/* toneset calculates the coefficients for tone.
      <cutoff> is -3db point in cps
      <flag> will reset history if set to 1
      <data> is an array of 3 doubles, used for bookeeping
   NOTE: REVERBIT doesn't try to use the inverse function cited below.
*/
static void
toneset(double SR, double cutoff, int flag, double data[])
{
   double x;

   x = 2.0 - cos(cutoff * PI2 / SR);        /* feedback coeff. */
   data[1] = x - sqrt(x * x - 1.0);      
   data[0] = 1.0 - data[1];                 /* gain coeff. */
   if (cutoff < 0.0)
      data[1] *= -1.0;                      /* inverse function */
   if (flag)
      data[2] = 0.0;
}


/* tone is a simple 1st order recursive lowpass filter
      <data> is an array of 3 doubles, used for bookeeping
*/
static double
tone(double sig, double data[])
{
   data[2] = data[0] * sig + data[1] * data[2];
   return data[2];
}


Instrument *makeREVERBIT()
{
   REVERBIT *inst;

   inst = new REVERBIT();
   inst->set_bus_config("REVERBIT");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("REVERBIT", makeREVERBIT);
}
#endif
