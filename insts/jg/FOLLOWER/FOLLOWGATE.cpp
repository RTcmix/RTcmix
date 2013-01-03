/* FOLLOWGATE - envelope follower controlling a gate

   FOLLOWGATE uses the amplitude envelope of the modulator to control the
   action of a gate applied to the carrier.  The gate opens when the power
   of the modulator rises above a threshold and closes when the power falls
   below the threshold.  The bottom of the gate can be adjusted (via the
   <range> parameter) so that it sits flush against the "floor" or rides
   above the floor, letting some signal through even when the gate is closed.
   The carrier is supplied as the "left" channel, the modulator as the
   "right" channel.  (See below.)

   p0  = output start time
   p1  = input start time (must be 0 for aux bus -- see below)
   p2  = duration
   p3  = carrier amplitude multiplier *
   p4  = modulator amplitude multiplier
   p5  = power gauge window length (in samples; try 100)
   p6  = smoothness -- how much to smooth the power gauge output (0-1; try .8)
   p7  = attack time -- how long it takes the gate to fully open once the
         modulator power rises above the threshold
   p8  = release time -- how long it takes the gate to fully close once the
         modulator power falls below the threshold
   p9  = pan (in percent-to-left form: 0-1) [optional, default is .5]
   p10 = power threshold [optional; if missing, must use gen 2] **
   p11 = range [optional; if missing, must use gen 3] ***

   p3 (carrier amp), p4 (modulator amp), p6 (smoothness), p7 (attack time),
   p8 (release time), p9 (pan), p10 (threshold) and p11 (range) can
   receive dynamic updates from a table or real-time control source.

   NOTES:

     -  The "left" input channel comes from the bus with the lower number;
        the "right" input channel from the bus with the higher number.

     -  Currently in RTcmix it's not possible for an instrument to take input
        from both an "in" bus and an "aux in" bus at the same time.  So, for
        example, if you want the modulator to come from a microphone, which
        must enter via an "in" bus, and the carrier to come from a WAVETABLE
        instrument via an "aux" bus, then you must route the mic into the MIX
        instrument as a way to convert it from "in" to "aux in".  If you want
        the carrier to come from a file, then it must first go through MIX 
        (or some other instrument) to send it into an aux bus.  Since the
        instrument is usually taking input from an aux bus, the input start
        time for this instrument must be zero.  The only exception would be if
        you're taking the carrier and modulator signals from the left and right
        channels of the same sound file.

     -  The envelope follower consists of a power gauge that measures the
        average power of the modulator signal.  The window length (p5) is the
        number of samples to average.  Large values (> 1000) track only gross
        amplitude changes; small values (< 10) track very minute changes.  If
        the power level changes abruptly, as it does especially with long
        windows, you'll hear zipper noise.  Reduce this by increasing the
        smoothness (p6).  This applies a low-pass filter to the power gauge
        signal, smoothing any abrupt changes.

     -  You'll probably always need to boost the modulator amplitude
        multiplier (p4) beyond what you'd expect, because we're using the
        RMS power of the modulator to affect the carrier, and this is always
        lower than the peak amplitude of the modulator signal.

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p3 (carrier amp), even if the latter is dynamic.

   ** If p10 is missing, you must use an old-style gen table 2 for the
   power threshold curve.

   *** If p11 is missing, you must use an old-style gen table 3 for the
   range curve.


   John Gibson <johgibso at indiana dot edu>, 1/5/03; rev for v4, JGG, 7/24/04
*/

//#define DEBUG
#include "FOLLOWGATE.h"
#include <float.h>   // for FLT_MAX


/* ------------------------------------------------------------ FOLLOWGATE -- */
FOLLOWGATE :: FOLLOWGATE()
{
   thresh_table = NULL;
   range_table = NULL;
   state = belowThreshold;
   attack_time = release_time = -FLT_MAX;
}


/* ----------------------------------------------------------- ~FOLLOWGATE -- */
FOLLOWGATE :: ~FOLLOWGATE()
{
   delete thresh_table;
   delete range_table;
   delete envelope;
}


/* -------------------------------------------------------------- pre_init -- */
int FOLLOWGATE :: pre_init(double p[], int n_args)
{
   oneoverSR = 1.0 / SR;
   return 0;
}


/* ------------------------------------------------------------- post_init -- */
int FOLLOWGATE :: post_init(double p[], int n_args)
{
   if (n_args < 11) {   // no p10 power threshold PField, must use gen table
      double *function = floc(2);
      if (function == NULL)
         return die(instname(), "Either use the power threshold pfield (p10) "
                                "or make an old-style gen function in slot 2.");
      int len = fsize(2);
      thresh_table = new TableL(SR, getdur(), function, len);
   }

   if (n_args < 12) {   // no p11 range PField, must use gen table
      double *function = floc(3);
      if (function == NULL)
         return die(instname(), "Either use the range pfield (p11) "
                                "or make an old-style gen function in slot 3.");
      int len = fsize(3);
      range_table = new TableL(SR, getdur(), function, len);
   }

   envelope = new Envelope(SR);

   return 0;
}


/* --------------------------------------------------------- update_params -- */
void FOLLOWGATE :: update_params(double p[])
{
   if (p[7] != attack_time) {
      attack_time = p[7];
      if (attack_time < 0.0)
         attack_time = 0.0;
      attack_rate = attack_time ? oneoverSR / attack_time : 1.0;
   }
   if (p[8] != release_time) {
      release_time = p[8];
      if (release_time < 0.0)
         release_time = 0.0;
      release_rate = release_time ? oneoverSR / release_time : 1.0;
   }

   pctleft = nargs > 9 ? p[9] : 0.5;         // default is center

   if (nargs > 10)
      threshold = p[10];
   else
      threshold = thresh_table->tick(currentFrame(), 1.0);
   if (threshold < 0.0)
      threshold = 0.0;

   if (nargs > 11)
      range = p[11];
   else
      range = range_table->tick(currentFrame(), 1.0);
   if (range < 0.0)
      range = 0.0;

   if (state == belowThreshold) {
      if (envelope->getState() == ENV_RAMPING)
         envelope->setTarget(range);
      else
         envelope->setValue(range);
   }
}


/* -------------------------------------------------------- process_sample -- */
float FOLLOWGATE :: process_sample(float sample, float power)
{
   if (power >= threshold) {
      if (state == belowThreshold) {
         state = aboveThreshold;
         envelope->setRate(attack_rate);
         envelope->keyOn();
      }
   }
   else {
      if (state == aboveThreshold) {
         state = belowThreshold;
         envelope->setRate(release_rate);
         envelope->keyOff();
         envelope->setTarget(range);
      }
   }

   float env = envelope->tick();

   DPRINT3("sample: %f, power: %f, env: %f\n", sample, power, env);

   return sample * env;
}


/* -------------------------------------------------------- makeFOLLOWGATE -- */
Instrument *makeFOLLOWGATE()
{
   FOLLOWGATE *inst;

   inst = new FOLLOWGATE();
   inst->set_bus_config("FOLLOWGATE");

   return inst;
}

#ifndef MAXMSP
/* ------------------------------------------------------------- rtprofile -- */
void rtprofile()
{
   RT_INTRO("FOLLOWGATE", makeFOLLOWGATE);
}
#endif

