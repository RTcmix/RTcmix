/* FOLLOWBUTTER - a simple envelope follower

   FOLLOWBUTTER maps the amplitude envelope of the modulator to the cutoff
   frequency of a filter applied to the carrier.  This filter works as
   described in the documentation for the BUTTER instrument, except that
   there is no balance parameter.  The carrier is supplied as the "left"
   channel, the modulator as the "right" channel.  (See below.)

   p0  = output start time
   p1  = input start time (must be 0 for aux bus -- see below)
   p2  = duration
   p3  = carrier amplitude multiplier
   p4  = modulator amplitude multiplier
   p5  = power gauge window length (in samples; try 100)
   p6  = smoothness -- how much to smooth the power gauge output (0-1; try .8)
   p7  = type of filter (1: lowpass, 2: highpass, 3: bandpass, 4: bandreject)
   p8  = minimum cutoff (or center) frequency
   p9  = maximum cutoff (or center) frequency
   p10 = steepness (> 0) [optional, default is 1]
   p11 = percent to left channel [optional, default is .5]

   Function table 1 is the overall amplitude envelope.
   Function table 2 is the bandwidth curve, if using the bandpass or
      bandreject filter types.

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

     -  You'll probably always need to boost the modulator amplitude
        multiplier (p4) beyond what you'd expect, because we're using
        the RMS power of the modulator to affect the carrier, and this
        is always lower than the peak amplitude of the modulator signal.

     -  Maximum cutoff (p9) is the cutoff frequency you get when the power
        gauge reads 1.0.  Whether the guage reaches -- or exceeds -- 1.0
        depends on the modulator amplitude multiplier, and is signal
        dependent.  In other words, just play around with the combination
        of max. cutoff and modulator amp. 

     -  Steepness (p10) is just the number of filters to add in series.
        Using more than 1 steepens the slope of the filter.  You may need
        to change p3 (carrier amp) to adjust for loss of power caused
        by connecting several filters in series.  Guard your ears!

   John Gibson <johgibso at indiana dot edu>, 8/7/03.
*/

//#define DEBUG
#include "FOLLOWBUTTER.h"


/* ---------------------------------------------------------- FOLLOWBUTTER -- */
FOLLOWBUTTER :: FOLLOWBUTTER()
{
   bwtable = NULL;
   curcf = 1.0;      // zeroes can result in nan's in setBandPass, etc.
   curbw = 1.0;
}


/* --------------------------------------------------------- ~FOLLOWBUTTER -- */
FOLLOWBUTTER :: ~FOLLOWBUTTER()
{
   for (int i = 0; i < nfilts; i++)
      delete filt[i];
   delete bwtable;
}


/* -------------------------------------------------------------- pre_init -- */
int FOLLOWBUTTER :: pre_init(float p[], int n_args)
{
   type = (FiltType) p[7];
   mincf = p[8];
   maxcf = p[9];
   nfilts = n_args > 10 ? (int)p[10] : 1;
   pctleft = n_args > 11 ? p[11] : 0.5;      /* default is center */

   if (mincf <= 0.0 || mincf >= maxcf)
      return die("FOLLOWBUTTER",
                 "Minimum cf (p8) must be greater than 0 and less "
                 "than the maximum cf.");
   if (maxcf > SR / 2.0)
      return die("FOLLOWBUTTER",
                  "Maximum cf (p9) must be less than the Nyquist frequency.");
   if (type != LowPass && type != HighPass && type != BandPass
         && type != BandReject)
      return die("FOLLOWBUTTER",
                 "Filter type must be 1 (lowpass), 2 (highpass), "
                 "3 (bandpass) or 4 (bandreject).");
   if (nfilts < 1 || nfilts > MAXFILTS)
      return die("FOLLOWBUTTER",
            "Steepness (p10) must be an integer between 1 and %d.", MAXFILTS);

   cfdiff = maxcf - mincf;

   return 0;
}


/* ------------------------------------------------------------- post_init -- */
int FOLLOWBUTTER :: post_init(float p[], int n_args)
{
   for (int i = 0; i < nfilts; i++)
      filt[i] = new Butter();

   if (type == BandPass || type == BandReject) {
      float *function = floc(2);
      if (function) {
         int len = fsize(2);
         bwtable = new TableL(dur, function, len);
      }
      else
         return die(instname(),
                           "You must create the bandwidth table (table 2).");
   }

   return 0;
}


/* --------------------------------------------------------- update_params -- */
void FOLLOWBUTTER :: update_params()
{
   if (bwtable) {
      float bw = bwtable->tick(currentFrame(), 1.0);
      if (bw < 0.0)
         bw *= -curcf;        /* percent of cf */
      if (bw != curbw) {
         if (type == BandPass)
            for (int i = 0; i < nfilts; i++)
               filt[i]->setBandPass(curcf, bw);
         else if (type == BandReject)
            for (int i = 0; i < nfilts; i++)
               filt[i]->setBandReject(curcf, bw);
         curbw = bw;
      }
   }
}


/* -------------------------------------------------------- process_sample -- */
float FOLLOWBUTTER :: process_sample(float sample, float power)
{
   int   i;

   DPRINT1("power: %f\n", power);

   float cf = mincf + (cfdiff * power);
   if (cf <= 0.0)
      cf = 1.0;
   else if (cf > SR * 0.5)
      cf = SR * 0.5;

   if (cf != curcf) {
      if (type == LowPass)
         for (i = 0; i < nfilts; i++)
            filt[i]->setLowPass(cf);
      else if (type == HighPass)
         for (i = 0; i < nfilts; i++)
            filt[i]->setHighPass(cf);
      else if (type == BandPass)
         for (i = 0; i < nfilts; i++)
            filt[i]->setBandPass(cf, curbw);
      else if (type == BandReject)
         for (i = 0; i < nfilts; i++)
            filt[i]->setBandReject(cf, curbw);
      curcf = cf;
      DPRINT1("cf: %f\n", cf);
   }

   for (i = 0; i < nfilts; i++)
      sample = filt[i]->tick(sample);

   return sample;
}


/* ------------------------------------------------------ makeFOLLOWBUTTER -- */
Instrument *makeFOLLOWBUTTER()
{
   FOLLOWBUTTER *inst;

   inst = new FOLLOWBUTTER();
   inst->set_bus_config("FOLLOWBUTTER");

   return inst;
}

/* ------------------------------------------------------------- rtprofile -- */
void rtprofile()
{
   RT_INTRO("FOLLOWBUTTER", makeFOLLOWBUTTER);
}


