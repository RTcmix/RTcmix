/* FOLLOWER - a simple envelope follower

   FOLLOWER applies the amplitude envelope of the modulator to the carrier.
   The carrier is supplied as the "left" channel, the modulator as the
   "right" channel.  (See below.)

   p0  = output start time
   p1  = input start time (must be 0 for aux bus -- see below)
   p2  = duration
   p3  = carrier amplitude multiplier *
   p4  = modulator amplitude multiplier
   p5  = power gauge window length (in samples; try 100)
   p6  = smoothness -- how much to smooth the power gauge output (0-1; try .8)
   p7  = percent to left channel [optional, default is .5]

   p3 (carrier amp), p4 (modulator amp), p6 (smoothness) and p7 (pan) can
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
        multiplier (p4) beyond what you'd expect, because we're using
        the RMS power of the modulator to affect the carrier, and this
        is always lower than the peak amplitude of the modulator signal.

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by the p3 carrier amplitude multiplier, even if the latter is dynamic.


   John Gibson (johgibso at indiana dot edu), 1/5/03; rev for v4, JGG, 7/24/04
*/

//#define DEBUG
#include "FOLLOWER.h"

/* This is such a simple envelope follower instrument that most of the
   work is done in the base class.
*/


/* -------------------------------------------------------------- FOLLOWER -- */
FOLLOWER :: FOLLOWER()
{
}


/* ------------------------------------------------------------- ~FOLLOWER -- */
FOLLOWER :: ~FOLLOWER()
{
}


/* -------------------------------------------------------------- pre_init -- */
int FOLLOWER :: pre_init(double p[], int n_args)
{
   return 0;
}


/* ------------------------------------------------------------- post_init -- */
int FOLLOWER :: post_init(double p[], int n_args)
{
   return 0;
}


/* --------------------------------------------------------- update_params -- */
void FOLLOWER :: update_params(double p[])
{
   pctleft = nargs > 7 ? p[7] : 0.5;      // default is center
}


/* -------------------------------------------------------- process_sample -- */
float FOLLOWER :: process_sample(float sample, float power)
{
   DPRINT1("%f\n", power);
   return sample * power;
}


/* ---------------------------------------------------------- makeFOLLOWER -- */
Instrument *makeFOLLOWER()
{
   FOLLOWER *inst;

   inst = new FOLLOWER();
   inst->set_bus_config("FOLLOWER");

   return inst;
}

#ifndef MAXMSP
/* ------------------------------------------------------------- rtprofile -- */
void rtprofile()
{
   RT_INTRO("FOLLOWER", makeFOLLOWER);
}
#endif

