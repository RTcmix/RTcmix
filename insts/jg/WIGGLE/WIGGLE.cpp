/* WIGGLE - wavetable oscillator with frequency modulation and filter

   This instrument is like WAVETABLE, except that it lets you change
   the pitch with a glissando curve and/or frequency modulation.  The
   modulation can be subaudio rate (for vibrato) or audio rate (for
   basic Chowning FM).  There is an optional filter, either lowpass or
   highpass.  Many of the parameters are time-varying, specified by
   gen functions.

   p0 = output start time
   p1 = duration
   p2 = carrier amplitude
   p3 = carrier oscillator frequency (or oct.pc if < 15)
   p4 = modulator depth control type (0: no modulation at all, 1: percent
        of carrier frequency, 2: modulation index) [optional, default is 0]
   p5 = type of filter (0: no filter, 1: low-pass, 2: high-pass)
        [optional, default is 0]
   p6 = steepness (> 0) [optional, default is 1]
   p7 = balance output and input signals (0:no, 1:yes) [optional, default is 0]

   p4 (modulator depth control type) tells the instrument how to interpret
   the values in the modulator depth function table.  You can express these
   as a percentage of the carrier (useful for subaudio rate modulation) or
   as a modulation index (useful for audio rate FM).  If you don't want
   to use the modulating oscillator at all, pass 0 for this pfield.  Then
   you don't need to create function tables 4-6.

   p6 (steepness) is just the number of filters to add in series.  Using more
   than 1 steepens the slope of the filter.  If you don't set p7 (balance)
   to 1, you'll need to change p2 (carrier amp) to adjust for loss of power
   caused by connecting several filters in series.

   p7 (balance) tries to adjust the output of the filter so that it has
   the same power as the input.  This means there's less fiddling around
   with p2 (amp) to get the right amplitude when steepness is > 1.  However,
   it has drawbacks: it can introduce a click at the start of the sound, it
   can cause the sound to pump up and down a bit, and it eats extra CPU time.

   Here are the function table assignments:

      1: amplitude curve (setline)
      2: carrier oscillator waveform (e.g., gen 9 or 10)
      3: carrier glissando curve (linear octave offsets from p3 frequency)
      4: modulator oscillator waveform
      5: modulator frequency (in Hz; or, if negative, ratio to carrier freq)
         E.g., "makegen(5, 18, 1000, 0,-2, 1,-2.3)" will change gradually from
         a C:M ratio of 1:2 to a ratio of 1:2.3 over the course of the note.
      6: modulator depth (p4 determines how these values are interpreted)
      7: filter cutoff frequency
      8: pan curve (from 0 to 1)

   NOTE: The glissando table is read without interpolation.  This was done
   on purpose, to permit sub-audio modulation with "jagged edges."
   For example, if you say
      makegen(3, 20, 1, 10)
   you'll get audible random stairsteps (with no transitions between the
   steps).

   John Gibson <johgibso at indiana dot edu>, 12/4/01.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "WIGGLE.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG1
//#define DEBUG2

#define AMP_FUNC             1
#define CAR_WAVE_FUNC        2
#define CAR_GLISS_FUNC       3
#define MOD_WAVE_FUNC        4
#define MOD_FREQ_FUNC        5
#define MOD_DEPTH_FUNC       6
#define FILTER_CF_FUNC       7
#define PAN_FUNC             8

#define BALANCE_WINDOW_SIZE  10


WIGGLE :: WIGGLE() : Instrument()
{
   /* might not use these, so make them NULL */
   balancer = NULL;
   modulator = NULL;
   amp_table = NULL;
   modfreq_table = NULL;
   moddepth_table = NULL;
   filtcf_table = NULL;
   pan_table = NULL;

   mod_depth = 0.0;        /* must init, in case depth_type == NoModOsc */

   branch = 0;
}


WIGGLE :: ~WIGGLE()
{
   for (int i = 0; i < nfilts; i++)
      delete filt[i];
   if (balancer)
      delete balancer;
   delete carrier;
   if (modulator)
      delete modulator;
   if (amp_table)
      delete amp_table;
   delete cargliss_table;
   if (modfreq_table)
      delete modfreq_table;
   if (moddepth_table)
      delete moddepth_table;
   if (filtcf_table)
      delete filtcf_table;
   if (pan_table)
      delete pan_table;
}


int WIGGLE :: init(double p[], int n_args)
{
   float outskip = p[0];
   float dur = p[1];
   amp = p[2];
   base_freq = p[3];
   if (base_freq < 15.0)
      base_freq = cpspch(base_freq);
   depth_type = n_args > 4 ? (int)p[4] : 0;
   filter_type = n_args > 5 ? (FiltType)p[5] : NoFilter;

   float ringdur;
   if (filter_type == NoFilter) {
      nfilts = do_balance = 0;
      ringdur = 0.0;
   }
   else {
      if (filter_type != LowPass && filter_type != HighPass)
         return die("WIGGLE", "Filter type (p5) must be 0, 1, or 2.");
      nfilts = n_args > 6 ? (int)p[6] : 1;
      if (nfilts < 1 || nfilts > MAXFILTS)
         return die("WIGGLE",
                    "Steepness (p6) must be an integer between 1 and %d.",
                    MAXFILTS);
      do_balance = n_args > 7 ? (int)p[7] : 0;
      if (do_balance) {
         balancer = new Balance(SR);
         balancer->setWindowSize(BALANCE_WINDOW_SIZE);
      }
      ringdur = 0.1;
   }

   if (rtsetoutput(outskip, dur + ringdur, this) == -1)
      return DONT_SCHEDULE;
   if (outputChannels() < 1 || outputChannels() > 2)
      return die("WIGGLE", "Output must be mono or stereo.");

   for (int i = 0; i < nfilts; i++)
      filt[i] = new Butter(SR);

   amp_array = floc(AMP_FUNC);
   if (amp_array) {
      int len = fsize(AMP_FUNC);
      amp_table = new TableL(SR, dur, amp_array, len);
   }
   else
      advise("WIGGLE", "Setting phrase curve to all 1's.");

   carwave_array = floc(CAR_WAVE_FUNC);
   if (carwave_array) {
      int len = fsize(CAR_WAVE_FUNC);
      carrier = new OscilL(SR, 0.0, carwave_array, len);
   }
   else
      return die("WIGGLE",
                 "You haven't made the carrier waveform function (table %d).",
                 CAR_WAVE_FUNC);

   cargliss_array = floc(CAR_GLISS_FUNC);
   if (cargliss_array) {
      int len = fsize(CAR_GLISS_FUNC);
      cargliss_table = new TableN(SR, dur, cargliss_array, len);
   }
   else
      return die("WIGGLE",
                 "You haven't made the carrier glissando function (table %d).",
                 CAR_GLISS_FUNC);

   if (depth_type != NoModOsc) {
      modwave_array = floc(MOD_WAVE_FUNC);
      if (modwave_array) {
         int len = fsize(MOD_WAVE_FUNC);
         modulator = new OscilL(SR, 0.0, modwave_array, len);
      }
      else
         return die("WIGGLE",
                 "You haven't made the modulator waveform function (table %d).",
                 MOD_WAVE_FUNC);

      modfreq_array = floc(MOD_FREQ_FUNC);
      if (modfreq_array) {
         int len = fsize(MOD_FREQ_FUNC);
         modfreq_table = new TableL(SR, dur, modfreq_array, len);
      }
      else
         return die("WIGGLE",
               "You haven't made the modulator frequency function (table %d).",
               MOD_FREQ_FUNC);

      moddepth_array = floc(MOD_DEPTH_FUNC);
      if (moddepth_array) {
         int len = fsize(MOD_DEPTH_FUNC);
         moddepth_table = new TableL(SR, dur, moddepth_array, len);
      }
      else
         return die("WIGGLE",
                    "You haven't made the modulator depth function (table %d).",
                    MOD_DEPTH_FUNC);
   }

   if (filter_type != NoFilter) {
      filtcf_array = floc(FILTER_CF_FUNC);
      if (filtcf_array) {
         int len = fsize(FILTER_CF_FUNC);
         filtcf_table = new TableL(SR, dur, filtcf_array, len);
      }
      else
         return die("WIGGLE",
                  "You haven't made the cutoff frequency function (table %d).",
                  FILTER_CF_FUNC);
   }

   if (outputChannels() == 2) {
      pan_array = floc(PAN_FUNC);
      if (pan_array) {
         int len = fsize(PAN_FUNC);
         pan_table = new TableL(SR, dur, pan_array, len);
      }
      else
         return die("WIGGLE", "You haven't made the pan function (table %d).",
                                                                     PAN_FUNC);
   }

   skip = (int)(SR / (float)resetval);
   aamp = amp;        /* in case amp_table == NULL */
   cpsoct10 = cpsoct(10.0);

   return nSamps();
}


int WIGGLE :: run()
{
   for (int i = 0; i < framesToRun(); i++) {
      if (--branch <= 0) {
         if (amp_table)
            aamp = amp_table->tick(currentFrame(), 1.0) * amp;
         car_gliss = cargliss_table->tick(currentFrame(), 1.0);
         car_gliss = cpsoct(10.0 + car_gliss) / cpsoct10;
         if (depth_type != NoModOsc) {
            mod_freq = modfreq_table->tick(currentFrame(), 1.0);
            mod_depth = moddepth_table->tick(currentFrame(), 1.0);
            if (depth_type == CarPercent)
               mod_depth *= 0.01;
         }
         if (filter_type != NoFilter) {
            float cf = filtcf_table->tick(currentFrame(), 1.0);
            if (cf <= 0.0)
               cf = 1.0;
            if (cf > SR * 0.5)
               cf = SR * 0.5;
            if (filter_type == LowPass) {
               for (int j = 0; j < nfilts; j++)
                  filt[j]->setLowPass(cf);
            }
            else if (filter_type == HighPass) {
               for (int j = 0; j < nfilts; j++)
                  filt[j]->setHighPass(cf);
            }
         }
         if (outputChannels() == 2)
            pctleft = pan_table->tick(currentFrame(), 1.0);

#ifdef DEBUG1
         printf("amp=%f gliss=%f mfreq=%f mdpth=%f pan=%f\n",
                  aamp, car_gliss, mod_freq, mod_depth, pctleft);
#endif
         branch = skip;
      }

      car_freq = base_freq * car_gliss;

      float mod_sig = 0.0;
      if (mod_depth) {
         float mfreq = mod_freq;
         if (mfreq < 0.0)     /* negative value acts as ratio flag */
            mfreq = -mfreq * car_freq;
         if (depth_type == CarPercent)
            mod_sig = modulator->tick(mfreq, mod_depth * car_freq);
         else {   /* ModIndex */
            float mdepth = mod_depth * mfreq;  /* convert mdepth to peak dev. */
            mod_sig = modulator->tick(mfreq, mdepth);
         }
      }
      float car_sig = carrier->tick(car_freq + mod_sig, aamp);
#ifdef DEBUG2
      printf("carfreq=%f carsig=%f modsig=%f\n", car_freq, car_sig, mod_sig);
#endif

      float sig = car_sig;
      for (int j = 0; j < nfilts; j++)
         sig = filt[j]->tick(sig);

      if (do_balance)
         sig = balancer->tick(sig, car_sig);

      float out[2];
      if (outputChannels() == 2) {
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


Instrument *makeWIGGLE()
{
   WIGGLE *inst;

   inst = new WIGGLE();
   inst->set_bus_config("WIGGLE");

   return inst;
}

void rtprofile()
{
   RT_INTRO("WIGGLE", makeWIGGLE);
}

