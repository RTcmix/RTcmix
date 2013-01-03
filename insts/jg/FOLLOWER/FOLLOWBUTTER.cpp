/* FOLLOWBUTTER - an envelope follower driving a Butterworth filter

   FOLLOWBUTTER maps the amplitude envelope of the modulator to the cutoff
   frequency of a filter applied to the carrier.  This filter works as
   described in the documentation for the BUTTER instrument, except that
   there is no balance parameter.  The carrier is supplied as the "left"
   channel, the modulator as the "right" channel.  (See below.)

   p0  = output start time
   p1  = input start time (must be 0 for aux bus -- see below)
   p2  = duration
   p3  = carrier amplitude multiplier *
   p4  = modulator amplitude multiplier
   p5  = power gauge window length (in samples; try 100)
   p6  = smoothness -- how much to smooth the power gauge output (0-1; try .8)
   p7  = type of filter ("lowpass", "highpass", "bandpass", "bandreject") **
   p8  = minimum cutoff (or center) frequency (Hz)
   p9  = maximum cutoff (or center) frequency (Hz)
   p10 = steepness (> 0) [optional, default is 1]
   p11 = pan (in percent-to-left form: 0-1) [optional, default is .5]
   p12 = filter bandwidth for bandpass/reject types (Hz if positive;
         if negative, the '-' sign acts as a flag to interpret the bw values
         as percentages (from 0 to 1) of the current cf.
         [optional; if missing, must use gen 2] ***

   p3 (carrier amp), p4 (modulator amp), p6 (smoothness), p7 (filter type),
   p8 (min. cutoff), p9 (max. cutoff), p11 (pan) and p12 (bandwidth) can
   receive dynamic updates from a table or real-time control source.
   p7 (filter type) can be updated only when using numeric codes. **

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

     -  Maximum cutoff (p9) is the cutoff frequency you get when the power
        gauge reads 1.0.  Whether the guage reaches -- or exceeds -- 1.0
        depends on the modulator amplitude multiplier, and is signal dependent.
        In other words, just play around with the combination of max. cutoff
        and modulator amp. 

     -  Steepness (p10) is the number of filters to add in series.  Using more
        than 1 steepens the slope of the filter.  You may need to change p3
        (carrier amp) to adjust for loss of power caused by connecting several
        filters in series.

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p3 (carrier amp), even if the latter is dynamic.

   ** You can also give numeric codes for the filter type (1: lowpass,
   2: highpass, 3: bandpass, 4: bandreject).  These can be changed during a
   note.  If you give the string version, you can't change types during a note.

   *** If p12 is missing, you must use an old-style gen table 2 for the
   filter bandwidth curve.


   John Gibson <johgibso at indiana dot edu>, 8/7/03; rev for v4, JGG, 7/24/04
*/

//#define DEBUG
#include "FOLLOWBUTTER.h"
#include <PField.h>
#include <float.h>   // for FLT_MAX


/* ---------------------------------------------------------- FOLLOWBUTTER -- */
FOLLOWBUTTER :: FOLLOWBUTTER()
{
   bwtable = NULL;
   cf = bw = -FLT_MAX;
   filttype_was_string = false;
}


/* --------------------------------------------------------- ~FOLLOWBUTTER -- */
FOLLOWBUTTER :: ~FOLLOWBUTTER()
{
   for (int i = 0; i < nfilts; i++)
      delete filt[i];
   delete bwtable;
}


/* ----------------------------------------------- getFiltType and friends -- */
static const char *_filttype_name[] = {
   "lowpass",     // 0
   "highpass",    // 1
   "bandpass",    // 2
   "bandreject",  // 3
   NULL
};

inline int _string_to_filtcode(const char *str)
{
   for (int i = 0; _filttype_name[i] != NULL; i++)
      if (strncmp(str, _filttype_name[i], 5) == 0)   // 5 is min. to distinguish
         return i;
   return -1;
}

FiltType FOLLOWBUTTER :: getFiltType(bool trystring)
{
   double index = (double) currentFrame() / nSamps();
   const PField &field = getPField(7);

   // must try int first, since a valid code cast to char * will crash strncmp
   int code = field.intValue(index) - 1;  // NB: user codes are 1-based
   if ((code < LowPass || code > BandReject) && trystring) {
      const char *str = field.stringValue(index);
      code = _string_to_filtcode(str);   // -1 if no match
      if (code != -1)
         filttype_was_string = true;
   }

   FiltType filttype;
   switch (code) {
      case 0: filttype = LowPass;      break;
      case 1: filttype = HighPass;     break;
      case 2: filttype = BandPass;     break;
      case 3: filttype = BandReject;   break;
      default: filttype = FiltInvalid; break;
   }
   return filttype;
}


/* -------------------------------------------------------------- pre_init -- */
int FOLLOWBUTTER :: pre_init(double p[], int n_args)
{
   nyquist = SR * 0.5;

   mincf = p[8];
   maxcf = p[9];
   nfilts = n_args > 10 ? (int) p[10] : 1;

   if (mincf < 1.0 || mincf > maxcf)
      return die(instname(), "The minimum cf (p8) must be greater than 1 but "
                             "not greater than the maximum cf.");
   if (maxcf > nyquist)
      return die(instname(), "The maximum cf (p9) must be less than the "
                             "Nyquist frequency.");

   if (nfilts < 1 || nfilts > MAXFILTS)
      return die(instname(), "Steepness (p10) must be an integer between 1 "
                             "and %d.", MAXFILTS);

   return 0;
}


/* ------------------------------------------------------------- post_init -- */
int FOLLOWBUTTER :: post_init(double p[], int n_args)
{
   type = getFiltType(true);
   if (type == FiltInvalid)
      return die(instname(), "Filter type must be \"lowpass\", \"highpass\", "
                             "\"bandpass\", or \"bandreject\".");

   for (int i = 0; i < nfilts; i++)
      filt[i] = new Butter(SR);

   if (type == BandPass || type == BandReject) {
      if (n_args < 13) {   // no p12 filter bandwidth PField, must use gen table
         double *function = floc(2);
         if (function == NULL)
            return die(instname(), "Either use the filter bandwidth pfield "
                        "(p12) or make an old-style gen function in slot 2.");
         int len = fsize(2);
         bwtable = new TableL(SR, getdur(), function, len);
      }
   }

   return 0;
}


/* --------------------------------------------------------- update_params -- */
void FOLLOWBUTTER :: update_params(double p[])
{
   // If init type spec was string, we don't allow type updates.
   if (!filttype_was_string) {
      FiltType newtype = getFiltType(false);
      if (newtype == FiltInvalid)
         newtype = LowPass;
      if (newtype != type) {
         type = newtype;
         cf = -FLT_MAX;             // force coefficient changes below
      }
   }

   mincf = p[8];
   maxcf = p[9];
   if (mincf < 1.0)
      mincf = 1.0;
   if (mincf > nyquist)
      mincf = nyquist;
   if (maxcf < 1.0)
      maxcf = 1.0;
   if (maxcf > nyquist)
      maxcf = nyquist;
   if (maxcf < mincf)
      maxcf = mincf;
   cfdiff = maxcf - mincf;

   pctleft = nargs > 11 ? p[11] : 0.5;       // default is center

   if (type == BandPass || type == BandReject) {
      float newbw;
      if (nargs > 12)
         newbw = p[12];
      else
         newbw = bwtable->tick(currentFrame(), 1.0);
      if (newbw != bw) {
         bw = newbw;
         cf = -FLT_MAX;    // force update in process_sample
      }
   }
}


/* -------------------------------------------------------- process_sample -- */
float FOLLOWBUTTER :: process_sample(float sample, float power)
{
   DPRINT1("power: %f\n", power);

   float newcf = mincf + (cfdiff * power);
   if (newcf < 1.0)
      newcf = 1.0;
   else if (newcf > nyquist)
      newcf = nyquist;

   if (newcf != cf) {
      cf = newcf;
      if (type == LowPass)
         for (int i = 0; i < nfilts; i++)
            filt[i]->setLowPass(cf);
      else if (type == HighPass)
         for (int i = 0; i < nfilts; i++)
            filt[i]->setHighPass(cf);
      else {
         float realbw = bw;
         if (realbw < 0.0) {
            if (realbw < -1.0)
               realbw = -1.0;
            realbw *= -cf;       // percent of cf
         }
         if (type == BandPass)
            for (int i = 0; i < nfilts; i++)
               filt[i]->setBandPass(cf, realbw);
         else  // type == BandReject
            for (int i = 0; i < nfilts; i++)
               filt[i]->setBandReject(cf, realbw);
      }
      DPRINT1("cf: %f\n", cf);
   }

   for (int i = 0; i < nfilts; i++)
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

#ifndef MAXMSP
/* ------------------------------------------------------------- rtprofile -- */
void rtprofile()
{
   RT_INTRO("FOLLOWBUTTER", makeFOLLOWBUTTER);
}
#endif

