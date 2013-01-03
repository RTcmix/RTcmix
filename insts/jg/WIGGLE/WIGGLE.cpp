/* WIGGLE - wavetable oscillator with frequency modulation and filter

   This instrument is a kind of combination of WAVETABLE and FMINST.
   The time-varying capabilities in the first version of WIGGLE are
   now possible with WAVETABLE and FMINST, so WIGGLE may no longer be
   worth the bother, especially since a lot of complexity resulted from
   making it backward-compatible with older scores while supporting the
   new features of RTcmix v4.  Here it is anyway...

   If you write new scores for it, use all 14 of the pfields described
   below, and make sure that there are no makegens in your score.  The
   documentation below does not say how WIGGLE operates with the old gen
   scheme, but it will still run old scores with no change in sound.

   The parameters marked with '*' can receive dynamic updates from a table
   or a real-time control source.

      p0  = output start time
      p1  = duration
    * p2  = carrier amplitude
    * p3  = carrier oscillator frequency - Hz or oct.pc (see note 1)
      p4  = modulator depth control type (0: no modulation at all, 1: percent
            of carrier frequency, 2: modulation index - see note 2)
      p5  = type of filter (0: no filter, 1: low-pass, 2: high-pass)
      p6  = steepness (> 0) - see note 3
      p7  = balance output and input signals (0:no, 1:yes) - see note 4
      p8  = carrier wavetable
      p9  = modulator wavetable
    * p10 = modulator frequency - see note 5
    * p11 = modulator depth - see note 2
    * p12 = lowpass filter cutoff frequency
    * p13 = pan (in percent-to-left form: 0-1)

   NOTES

   1. Oct.pc format generally will not work as you expect for p3 (car freq)
      if the pfield changes dynamically.  Use Hz instead in that case.

   2. The modulator depth control type (p4) tells WIGGLE how to interpret 
      modulator depth values (p11).  You can express these as a percentage of
      the carrier (0-100), useful for subaudio rate modulation, or as a
      modulation index (useful for audio rate FM).  If you don't want to use
      the modulating oscillator at all, pass 0 for this pfield.

   3. Steepness (p6) is just the number of filters to add in series.  Using more
      than 1 steepens the slope of the filter.  If you don't set p7 (balance)
      to 1, you'll need to change p2 (carrier amp) to adjust for loss of power
      caused by connecting several filters in series.

   4. Balance (p7) tries to adjust the output of the filter so that it has
      the same power as the input.  This means there's less fiddling around
      with p2 (amp) to get the right amplitude when steepness is > 1.  However,
      it has drawbacks: it can introduce a click at the start of the sound, it
      can cause the sound to pump up and down a bit, and it eats extra CPU time.

   5. Modulator frequency is in Hz.  Or, if it is negative, then its absolute
      value will be interpreted as the modulator's ratio to the carrier
      frequency.  E.g...

         modfreq = maketable("line", "nonorm", 100, 0,-2, 1,-2.3)

      will change gradually from a C:M ratio of 1:2 to a ratio of 1:2.3 over
      the course of the note.

   6. The carrier and modulator wavetables can be updated in real time using
      modtable(..., "draw", ...).


   John Gibson <johgibso at indiana dot edu>, 12/4/01; rev. for v4, 6/17/05.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <ugens.h>
#include <Instrument.h>
#include "WIGGLE.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG1
//#define DEBUG2

// NOTE: We support all the gen tables used with WIGGLE prior to v4.  The aim
// is to let this version of WIGGLE play old scores without any changes in
// sound.  If someone combines gens with v4 dynamic pfields, we do the best we
// can: we either scale the dynamic pfield value by the gen table value (e.g.,
// amp), or one overrides the other (e.g., pan).  But the inst really is not
// designed to handle both makegens and dynamic pfields in the same score.

#define AMP_FUNC             1
#define CAR_WAVE_FUNC        2
#define CAR_GLISS_FUNC       3
#define MOD_WAVE_FUNC        4
#define MOD_FREQ_FUNC        5
#define MOD_DEPTH_FUNC       6
#define FILTER_CF_FUNC       7
#define PAN_FUNC             8

#define BALANCE_WINDOW_SIZE  10


WIGGLE::WIGGLE() : Instrument()
{
   do_balance = false;
   carwave_array = NULL;
   modwave_array = NULL;
   balancer = NULL;
   modulator = NULL;
   amp_table = NULL;
   cargliss_table = NULL;
   modfreq_table = NULL;
   moddepth_table = NULL;
   filtcf_table = NULL;
   pan_table = NULL;

   car_freq_raw = -FLT_MAX;
   cf_raw = -FLT_MAX;
   mod_depth = 0.0;        // must init, in case depth_type == NoModOsc
   nyquist = SR * 0.5;

   branch = 0;
}


WIGGLE::~WIGGLE()
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
   if (cargliss_table)
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


DepthType WIGGLE::getDepthType(double pval)
{
   int intval = int(pval);
   DepthType type = NoModOsc;

   switch (intval) {
      case 0:
         type = NoModOsc;
         break;
      case 1:
         type = CarPercent;
         break;
      case 2:
         type = ModIndex;
         break;
      default:
         die("WIGGLE", "Invalid depth type %d\n.", intval);
         break;
   }
   return type;
}

FiltType WIGGLE::getFiltType(double pval)
{
   int intval = int(pval);
   FiltType type = NoFilter;

   switch (intval) {
      case 0:
         type = NoFilter;
         break;
      case 1:
         type = LowPass;
         break;
      case 2:
         type = HighPass;
         break;
      default:
         die("WIGGLE", "Invalid filter type %d\n.", intval);
         break;
   }
   return type;
}

int WIGGLE::init(double p[], int n_args)
{
   const float outskip = p[0];
   const float dur = p[1];
   depth_type = (n_args > 4) ? getDepthType(p[4]) : NoModOsc;
   filter_type = (n_args > 5) ? getFiltType(p[5]) : NoFilter;

   float ringdur;
   if (filter_type == NoFilter) {
      nfilts = 0;
      ringdur = 0.0f;
   }
   else {
      if (filter_type != LowPass && filter_type != HighPass)
         return die("WIGGLE", "Filter type (p5) must be 0, 1, or 2.");
      nfilts = (n_args > 6) ? int(p[6]) : 1;
      if (nfilts < 1 || nfilts > MAXFILTS)
         return die("WIGGLE",
                    "Steepness (p6) must be an integer between 1 and %d.",
                    MAXFILTS);
      if (n_args > 7)
         do_balance = bool(p[7]);
      if (do_balance) {
         balancer = new Balance(SR);
         balancer->setWindowSize(BALANCE_WINDOW_SIZE);
      }
      ringdur = 0.1f;
   }

   if (rtsetoutput(outskip, dur + ringdur, this) == -1)
      return DONT_SCHEDULE;
   if (outputChannels() < 1 || outputChannels() > 2)
      return die("WIGGLE", "Output must be mono or stereo.");

   for (int i = 0; i < nfilts; i++)
      filt[i] = new Butter(SR);

   double *array = floc(AMP_FUNC);
   if (array) {
      int len = fsize(AMP_FUNC);
      amp_table = new TableL(SR, dur, array, len);
   }

   int len;
   if (n_args > 8)
      carwave_array = (double *) getPFieldTable(8, &len);
   if (carwave_array == NULL) {
      carwave_array = floc(CAR_WAVE_FUNC);
      if (carwave_array == NULL)
         return die("WIGGLE", "Either use the carrier wavetable pfield (p8), "
                              "or make an old-style gen function in slot %d.",
                              CAR_WAVE_FUNC);
      len = fsize(CAR_WAVE_FUNC);
   }
   carrier = new OscilL(SR, 0.0, carwave_array, len);

   array = floc(CAR_GLISS_FUNC);
   if (array) {
      len = fsize(CAR_GLISS_FUNC);
      cargliss_table = new TableN(SR, dur, array, len);
   }

   if (depth_type != NoModOsc) {
      if (n_args > 9)
         modwave_array = (double *) getPFieldTable(9, &len);
      if (modwave_array == NULL) {
         modwave_array = floc(MOD_WAVE_FUNC);
         if (modwave_array == NULL)
            return die("WIGGLE", "Either use the modulator wavetable pfield "
                                 "(p9), or make an old-style gen function "
                                 "in slot %d.", MOD_WAVE_FUNC);
         len = fsize(MOD_WAVE_FUNC);
      }
      modulator = new OscilL(SR, 0.0, modwave_array, len);

      array = floc(MOD_FREQ_FUNC);
      if (array) {
         len = fsize(MOD_FREQ_FUNC);
         modfreq_table = new TableL(SR, dur, array, len);
      }
      else if (n_args < 11)    // no p10 mod freq
         return die("WIGGLE", "Either use the modulator frequency pfield "
                              "(p10), or make an old-style gen function in "
                              "slot %d.", MOD_FREQ_FUNC);

      array = floc(MOD_DEPTH_FUNC);
      if (array) {
         len = fsize(MOD_DEPTH_FUNC);
         moddepth_table = new TableL(SR, dur, array, len);
      }
      else if (n_args < 12)    // no p11 mod depth
         return die("WIGGLE", "Either use the modulator depth pfield "
                              "(p11), or make an old-style gen function in "
                              "slot %d.", MOD_DEPTH_FUNC);
   }

   if (filter_type != NoFilter) {
      array = floc(FILTER_CF_FUNC);
      if (array) {
         len = fsize(FILTER_CF_FUNC);
         filtcf_table = new TableL(SR, dur, array, len);
      }
      else if (n_args < 13)    // no p12 filter cf
         return die("WIGGLE", "Either use the filter cutoff frequency pfield "
                              "(p12), or make an old-style gen function in "
                              "slot %d.", FILTER_CF_FUNC);
   }

   if (outputChannels() == 2) {
      array = floc(PAN_FUNC);
      if (array) {
         len = fsize(PAN_FUNC);
         pan_table = new TableL(SR, dur, array, len);
      }
      else if (n_args < 14)    // no p13 pan
         return die("WIGGLE", "Either use the pan pfield (p13), or make an "
                              "old-style gen function in slot %d.", PAN_FUNC);
   }

   cpsoct10 = cpsoct(10.0);

   return nSamps();
}


void WIGGLE::doupdate()
{
   double p[14];
   update(p, 14, 1 << 2 | 1 << 3 | 1 << 8 | 1 << 9 | 1 << 10 | 1 << 11
                                                   | 1 << 12 | 1 << 13);

   amp = p[2];
   if (amp_table)
      amp *= amp_table->tick(currentFrame(), 1.0);

   if (p[3] != car_freq_raw) {
      car_freq_raw = p[3];
      if (car_freq_raw < 15.0f)
         base_car_freq = cpspch(car_freq_raw);
      else
         base_car_freq = car_freq_raw;
   }

   // back-compat for gliss gen function
   if (cargliss_table) {
      float gliss = cargliss_table->tick(currentFrame(), 1.0);
      gliss = cpsoct(10.0 + gliss) / cpsoct10;
      car_freq = base_car_freq * gliss;
   }
   else
      car_freq = base_car_freq;

   if (depth_type != NoModOsc) {
      // guaranteed to have either the gen or the pfield
      mod_freq = modfreq_table ? modfreq_table->tick(currentFrame(), 1.0)
                                                                     : p[10];
      if (mod_freq < 0.0f)     // negative value acts as ratio flag
         mod_freq *= -car_freq;

      // guaranteed to have either the gen or the pfield
      mod_depth = moddepth_table ? moddepth_table->tick(currentFrame(), 1.0)
                                                                     : p[11];
      if (depth_type == CarPercent)
         mod_depth *= 0.01f;
   }

   if (filter_type != NoFilter) {
      // guaranteed to have either the gen or the pfield
      float cf = filtcf_table ? filtcf_table->tick(currentFrame(), 1.0) : p[12];
      if (cf != cf_raw) {
         if (cf <= 0.0f)
            cf = 1.0f;
         if (cf > nyquist)
            cf = nyquist;
         if (filter_type == LowPass) {
            for (int i = 0; i < nfilts; i++)
               filt[i]->setLowPass(cf);
         }
         else if (filter_type == HighPass) {
            for (int i = 0; i < nfilts; i++)
               filt[i]->setHighPass(cf);
         }
         cf_raw = cf;
      }
   }

   pan = pan_table ? pan = pan_table->tick(currentFrame(), 1.0) : p[13];

#ifdef DEBUG1
   printf("amp=%f cfreq=%f mfreq=%f mdpth=%f pan=%f\n",
            amp, car_freq, mod_freq, mod_depth, pan);
#endif
}


int WIGGLE::run()
{
   const int nframes = framesToRun();

   for (int i = 0; i < nframes; i++) {
      if (--branch <= 0) {
         doupdate();
         branch = getSkip();
      }

      float mod_sig = 0.0f;
      if (mod_depth != 0.0f) {
         if (depth_type == CarPercent)
            mod_sig = modulator->tick(mod_freq, mod_depth * car_freq);
         else {   // ModIndex
            float mdepth = mod_depth * mod_freq;  // convert mdepth to peak dev.
            mod_sig = modulator->tick(mod_freq, mdepth);
         }
      }
      float car_sig = carrier->tick(car_freq + mod_sig, amp);
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
         out[0] = sig * pan;
         out[1] = sig * (1.0f - pan);
      }
      else
         out[0] = sig;

      rtaddout(out);
      increment();
   }

   return nframes;
}


Instrument *makeWIGGLE()
{
   WIGGLE *inst = new WIGGLE();
   inst->set_bus_config("WIGGLE");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("WIGGLE", makeWIGGLE);
}
#endif
