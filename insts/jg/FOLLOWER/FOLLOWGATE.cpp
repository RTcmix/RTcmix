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
   p3  = carrier amplitude multiplier
   p4  = modulator amplitude multiplier
   p5  = power gauge window length (in samples; try 100)
   p6  = smoothness -- how much to smooth the power gauge output (0-1; try .8)
   p7  = attack time -- how long it takes the gate to fully open once the
            modulator power rises above the threshold
   p8  = release time -- how long it takes the gate to fully close once the
            modulator power falls below the threshold
   p9  = percent to left channel [optional, default is .5]

   Function table 1 is the overall amplitude envelope.
   Function table 2 is the power threshold (try gen 18).
   Function table 3 is the range (try gen 18).

   NOTES:

     -  The "left" input channel comes from the bus with the lower number;
        the "right" input channel from the bus with the higher number.

     -  Currently in RTcmix it's not possible for an instrument to take
        input from both an "in" bus and an "aux in" bus at the same time.
        So, for example, if you want the modulator to come from a microphone,
        which must enter via an "in" bus, and the carrier to come from a
        WAVETABLE instrument via an "aux" bus, then you must route the
        mic into the MIX instrument as a way to convert it from "in" to
        "aux in".  If you want the carrier to come from a file, then it
        must first go through MIX (or some other instrument) to send it
        into an aux bus.  Since the instrument is usually taking input
        from an aux bus, the input start time for this instrument must be
        zero.  The only exception would be if you're taking the carrier
        and modulator signals from the left and right channels of the same
        sound file.

     -  The envelope follower consists of a power gauge that measures the
        average power of the modulator signal.  The window length (p5) is
        the number of samples to average.  Large values (> 1000) track only
        gross amplitude changes; small values (< 10) track very minute
        changes.  If the power level changes abruptly, as it does especially
        with long windows, you'll hear zipper noise.  Reduce this by
        increasing the smoothness (p6).  This applies a low-pass filter
        to the power gauge signal, smoothing any abrupt changes.

   John Gibson (johgibso@indiana.edu), 1/5/03.
*/

//#define DEBUG
#include "FOLLOWGATE.h"


/* ------------------------------------------------------------ FOLLOWGATE -- */
FOLLOWGATE :: FOLLOWGATE()
{
   state = belowThreshold;
}


/* ----------------------------------------------------------- ~FOLLOWGATE -- */
FOLLOWGATE :: ~FOLLOWGATE()
{
   delete thresh_table;
   delete range_table;
   delete envelope;
}


/* -------------------------------------------------------------- pre_init -- */
int FOLLOWGATE :: pre_init(float p[], int n_args)
{
   float attack_time = p[7];
   float release_time = p[8];
   pctleft = n_args > 9 ? p[9] : 0.5;     /* default is center */

   if (attack_time < 0.0)
      die(instname(), "Attack time must be positive.");
   if (release_time < 0.0)
      die(instname(), "Release time must be positive.");

   attack_rate = attack_time ? (1.0 / SR) / attack_time : 1.0;
   release_rate = release_time ? (1.0 / SR) / release_time : 1.0;

   return 0;
}


/* ------------------------------------------------------------- post_init -- */
int FOLLOWGATE :: post_init(float p[], int n_args)
{
   float *function = floc(2);
   if (function) {
      int len = fsize(2);
      thresh_table = new TableL(dur, function, len);
   }
   else
      die(instname(), "You must create the threshold table (table 2).");

   function = floc(3);
   if (function) {
      int len = fsize(3);
      range_table = new TableL(dur, function, len);
   }
   else
      die(instname(), "You must create the range table (table 3).");

   envelope = new Envelope();

   return 0;
}


/* --------------------------------------------------------- update_params -- */
void FOLLOWGATE :: update_params()
{
   threshold = thresh_table->tick(CurrentFrame(), 1.0);
   if (threshold < 0.0)
      threshold = 0.0;

   range = range_table->tick(CurrentFrame(), 1.0);
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
   DPRINT1("%f\n", power);

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
   DPRINT1("%f\n", env);

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


/* ------------------------------------------------------------- rtprofile -- */
void rtprofile()
{
   RT_INTRO("FOLLOWGATE", makeFOLLOWGATE);
}


