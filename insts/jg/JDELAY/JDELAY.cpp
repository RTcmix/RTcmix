/* JDELAY - regenerating delay instrument, adapted from DELAY

   Compared with DELAY, JDELAY adds:
      - a simple low-pass filter
      - control over wet/dry mix
      - a DC blocking filter
      - switchable pre/post-fader delay
  
   Parameters:
      p0 = output start time
      p1 = input start time
      p2 = input duration
      p3 = amplitude multiplier
      p4 = delay time
      p5 = delay feedback (i.e., regeneration multiplier) [-1 to 1]
      p6 = ring-down duration
      p7 = cutoff freq for low-pass filter (in cps)  (0 to disable filter)
      p8 = wet/dry mix (0: dry -> 1: wet)
      p9 = input channel number [optional, default is 0]
      p10 = pan (in percent-to-left form: 0-1) [optional, default is .5] 
      p11 = pre-fader send (0: No, 1: Yes) [optional, default is No]
      p12 = apply DC blocking filter (0: No, 1: Yes) [optional, default is Yes]
            (DC bias can affect sounds made with high feedback setting.)
  
   p3 (amplitude), p4 (delay time), p5 (feedback), p7 (cutoff), p8 (wet/dry)
   and p8 (pan) can receive dynamic updates from a table or real-time control
   source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   The point of the ring-down duration parameter is to let you control
   how long the delay will sound after the input has stopped.  Too short
   a time, and the sound may be cut off prematurely.

   If pre-fader send is set to 1, sends input signal to delay line with
   no attenuation.  Then p3 (amp multiplier) and setline controls entire
   note, including delay ring-down.
  
   John Gibson (jgg9c@virginia.edu), 6/23/99; rev for v4, 7/21/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include "JDELAY.h"
#include <rt.h>
#include <rtdefs.h>

// local functions
static void toneset(double, double, int, double []);
static double tone(double, double []);


JDELAY::JDELAY() : Instrument()
{
   in = NULL;
   delay = NULL;
   branch = 0;
   usefilt = false;
   warn_deltime = true;
}


JDELAY::~JDELAY()
{
   delete [] in;
   delete delay;
}


int JDELAY::init(double p[], int n_args)
{
   nargs = n_args;
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   amp = p[3];
   float deltime = p[4];
   regen = p[5];
   float ringdur = p[6];
   cutoff = p[7];
   percent_wet = p[8];
   inchan = n_args > 9 ? (int) p[9] : 0;
   prefadersend = n_args > 11 ? (bool) p[11] : false;    // default is "no"
   dcblock = n_args > 12 ? (bool) p[12] : true;          // default is "yes"

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   if (rtsetoutput(outskip, dur + ringdur, this) == -1)
      return DONT_SCHEDULE;
   insamps = (int) (dur * SR + 0.5);

   if (inchan >= inputChannels())
      return die("JDELAY", "You asked for channel %d of a %d-channel file.",
                                                      inchan, inputChannels());

   if (regen < -1.0 || regen > 1.0)
      return die("JDELAY", "Regeneration multiplier must be between -1 and 1.");

   if (cutoff < 0.0)
      return die("JDELAY",
              "Cutoff freq. should be positive (or zero to disable filter).");
   else if (cutoff == 0.0)
      rtcmix_advise("JDELAY", "Low-pass filter disabled.");
   else {
      usefilt = true;
      toneset(SR, cutoff, 1, tonedata);
   }

   if (percent_wet < 0.0 || percent_wet > 1.0)
      return die("JDELAY", "Wet/dry mix must be between 0 and 1 inclusive.");

   if (deltime <= 0.0)
      return die("JDELAY", "Illegal delay time (%g seconds)");

   long delsamps = (long) (deltime * SR + 0.5);
   delay = new Odelayi(delsamps);
   if (delay->length() == 0)
      return die("JDELAY", "Can't allocate delay line memory.");

   amptable = floc(1);
   if (amptable) {
      int amplen = fsize(1);
      if (prefadersend)
         tableset(SR, dur + ringdur, amplen, amptabs);
      else
         tableset(SR, dur, amplen, amptabs);
   }

   skip = (int) (SR / (float) resetval);

   prev_in = prev_out = 0.0;         // for DC-blocker

   return nSamps();
}


void JDELAY::doupdate()
{
   double p[11];

   if (prefadersend) {
      update(p, 11, kAmp | kDelTime | kDelRegen | kCutoff | kWetPercent | kPan);
      amp = p[3];
   }
   else {
      update(p, 11, kDelTime | kDelRegen | kCutoff | kWetPercent | kPan);
      amp = update(3, insamps);
   }
   if (amptable)
      amp *= tablei(currentFrame(), amptable, amptabs);

   float deltime = p[4];
   delsamps = deltime * SR;

   regen = p[5];
   if (regen < -1.0)
      regen = -1.0;
   else if (regen > 1.0)
      regen = 1.0;

   if (usefilt && p[7] != cutoff) {
      cutoff = p[7];
      if (cutoff <= 0.0)
         cutoff = 0.1;
      toneset(SR, cutoff, 0, tonedata);
   }

   percent_wet = p[8];
   if (percent_wet < 0.0)
      percent_wet = 0.0;
   else if (percent_wet > 1.0)
      percent_wet = 1.0;

   pctleft = nargs > 10 ? p[10] : 0.5;        // default is center
}


int JDELAY::configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int JDELAY::run()
{
   int samps = framesToRun() * inputChannels();

   if (currentFrame() < insamps)
      rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         doupdate();
         branch = skip;
      }

      float delsig = delay->getsamp(delsamps) * regen;

      float insig;
      if (currentFrame() < insamps)          // still taking input from file
         insig = in[i + inchan];
      else
         insig = 0.0;

      float out[2];
      if (prefadersend) {
         delay->putsamp(insig + delsig);
         out[0] = insig * (1.0 - percent_wet) + delsig * percent_wet;
         out[0] *= amp;
      }
      else {
         insig *= amp;
         delay->putsamp(insig + delsig);
         out[0] = insig * (1.0 - percent_wet) + delsig * percent_wet;
      }

      /* NOTE: We filter the sig *after* putting it in the delay line.
               Otherwise, pitch for short delays drops with low cutoff freqs.
      */
      if (usefilt)
         out[0] = tone(out[0], tonedata);

      /* The DC-blocking code is from Perry Cook's STK (STK98v2/DCBlock.cpp).
         It works pretty well (and quickly) for all but extreme cases. But I've
         gotten JDELAY to generate files with a fair amount of DC even *with*
         this simple filter. The Remove DC function in mxv works better. It's
         the standard cmix elliptical filter with 60db atten., stopband at 5 hz,
         and passband at 20 hz. But that's overkill for here.
      */
      if (dcblock) {
         float tmp_in = out[0];
         out[0] = tmp_in - prev_in + (0.99 * prev_out);
         prev_in = tmp_in;
         prev_out = out[0];
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


/* -------------------------------------------------------------------------- */
/* This filter nabbed from Doug Scott's place program */

/* toneset calculates the coefficients for tone.
      <cutoff> is -3db point in cps
      <flag> will reset history if set to 1
      <data> is an array of 3 doubles, used for bookeeping
   NOTE: JDELAY doesn't try to use the inverse function cited below.
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


Instrument *makeJDELAY()
{
   JDELAY *inst;

   inst = new JDELAY();
   inst->set_bus_config("JDELAY");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("JDELAY", makeJDELAY);
}
#endif
