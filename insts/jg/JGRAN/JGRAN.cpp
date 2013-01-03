/* JGRAN - granular synthesis with FM or AS grains

   p0  = output start time
   p1  = duration
   p2  = amplitude multiplier [See Note 1.]
   p3  = random seed (any integer; if 0, seed from system clock) [default: 0]
   p4  = oscillator configuration (0: additive, 1: FM) [default: 0]
   p5  = randomize oscillator starting phase (0: no, 1: yes) [default: yes]

   The following pfields replace old-style gens in v3.7 [See Note 2.]

   p6  = reference to grain envelope table [if missing, must use gen 2]
   p7  = reference to grain waveform table [if missing, must use gen 3]
   p8  = modulation frequency multiplier [if missing, can use gen 4]
   p9  = index of modulation [if missing, can use gen 5]
   p10 = minimum grain frequency [if missing, must use gen 6]
   p11 = maximum grain frequency [if missing, must use gen 7]
   p12 = minimum grain speed [if missing, must use gen 8]
   p13 = maximum grain speed [if missing, must use gen 9]
   p14 = minimum grain intensity [if missing, must use gen 10]
   p15 = maximum grain intensity [if missing, must use gen 11]
   p16 = grain density [if missing, must use gen 12]
   p17 = grain pan (in percent-to-left form: 0-1) [if missing, can use gen 13]
   p18 = grain pan randomization [if missing, can use gen 14]

   p2 (amplitude), p8 (mod. freq.), p9 (mod. index), p10 (min. grain freq.),
   p11 (max. grain freq.), p12 (min. grain speed), p13 (max. grain speed),
   p14 (min. grain intensity), p15 (max. grain intensity), p16 (grain density),
   p17 (grain pan) and p18 (grain pan randomization) can receive dynamic
   updates from a table or real-time control source.

   JGRAN produces only one stream of non-overlapping grains. To get more
   streams, call JGRAN more than once (maybe with different seeds).  Grains
   within one stream (note) never overlap.  (Use Mara Helmuth's SGRAN for this.)

   ----

   Notes about backward compatibility with pre-v4 scores:

   [1] If an old-style gen table 1 is present, its values will be multiplied
       by p2 (amplitude), even if the latter is dynamic.

   [2] If this pfield is missing, you must use an old-style gen table in the
       specified gen slot instead.  For pfields whose comment says "can use"
       rather than "must use", the JGRAN provides a default if missing.


   JGRAN was derived from a Cecilia module (StochasticGrains) by Mathieu
   Bezkorowajny and Jean Piche.

   John Gibson (johngibson@virginia.edu), 4/15/00; rev for v4, JGG, 7/25/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include "JGRAN.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG


JGRAN :: JGRAN() : Instrument()
{
   amp_table = modmult_table = modindex_table = minfreq_table = maxfreq_table
      = minspeed_table = maxspeed_table = minintens_table = maxintens_table
      = density_table = pan_table = panvar_table = NULL;

   grainenv_oscil = NULL;
   car_oscil = mod_oscil = NULL;
   durnoi = freqnoi = pannoi = ampnoi = phasenoi = NULL;

   branch = gsampcount = 0;
}


JGRAN :: ~JGRAN()
{
   delete amp_table;
   delete grainenv_oscil;
   delete modmult_table;
   delete modindex_table;
   delete minfreq_table;
   delete maxfreq_table;
   delete minspeed_table;
   delete maxspeed_table;
   delete minintens_table;
   delete maxintens_table;
   delete density_table;
   delete pan_table;
   delete panvar_table;
   delete car_oscil;
   delete mod_oscil;
   delete durnoi;
   delete freqnoi;
   delete pannoi;
   delete ampnoi;
   delete phasenoi;
}


inline TableL *
make_table(int function_num, float dur, double SR)
{
   TableL *table = NULL;

   double *tab = floc(function_num);
   if (tab) {
      int len = fsize(function_num);
      table = new TableL(SR, dur, tab, len);      
   }

   return table;
}


int JGRAN :: init(double p[], int n_args)
{
   nargs = n_args;
   float outskip = p[0];
   float dur = p[1];
   int seed = (int) p[3];
   osctype = (p[4] == 0.0) ? AS : FM;
   randomize_phase = n_args > 5 ? (bool) p[5] : true;    // default: yes

   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;
   if (outputChannels() > 2)
      return die("JGRAN", "Output must be mono or stereo.");

   amp_table = make_table(1, dur, SR);

   // get grain envelope table
   double *function = NULL;
   int tablelen = 0;
	if (n_args > 6) {       // handle table coming in as optional p6 TablePField
		function = (double *) getPFieldTable(6, &tablelen);
	}
	if (function == NULL) {
		function = floc(2);
		if (function == NULL)
			return die("JGRAN", "Either use the grain envelope pfield (p6) "
                    "or make an old-style gen function in slot 2.");
		tablelen = fsize(2);
	}
   grainenv_oscil = new OscilL(SR, 0.0, function, tablelen);

   // get grain waveform table and create oscillator(s)
   function = NULL;
   tablelen = 0;
	if (n_args > 7) {       // handle table coming in as optional p7 TablePField
		function = (double *) getPFieldTable(7, &tablelen);
	}
	if (function == NULL) {
		function = floc(3);
		if (function == NULL) {
         tablelen = DEFAULT_WAVETABLE_SIZE;
         rtcmix_advise("JGRAN", "Using sine for grain waveform (no table 3).");
      }
      else
		   tablelen = fsize(3);
	}
   car_oscil = new OscilL(SR, 0.0, function, tablelen);
   if (osctype == FM)
      mod_oscil = new OscilL(SR, 0.0, function, tablelen);

   // create additional tables, if corresponding pfield is missing
   if (osctype == FM) {
      if (n_args <= 8) {
         modmult_table = make_table(4, dur, SR);
         if (modmult_table == NULL)
            return die("JGRAN", "Either use the modulation frequency "
                                "multiplier pfield (p8) or make an old-style "
                                "gen function in slot 4.");
      }
      if (n_args <= 9) {
         modindex_table = make_table(5, dur, SR);
         if (modindex_table == NULL)
            return die("JGRAN", "Either use the index envelope pfield (p9) "
                                "or make an old-style gen function in slot 5.");
      }
   }
   if (n_args <= 10) {
      minfreq_table = make_table(6, dur, SR);
      if (minfreq_table == NULL)
         return die("JGRAN", "Either use the min. grain frequency pfield (p10) "
                             "or make an old-style gen function in slot 6.");
   }
   if (n_args <= 11) {
      maxfreq_table = make_table(7, dur, SR);
      if (maxfreq_table == NULL)
         return die("JGRAN", "Either use the max. grain frequency pfield (p11) "
                             "or make an old-style gen function in slot 7.");
   }
   if (n_args <= 12) {
      minspeed_table = make_table(8, dur, SR);
      if (minspeed_table == NULL)
         return die("JGRAN", "Either use the min. grain speed pfield (p12) "
                             "or make an old-style gen function in slot 8.");
   }
   if (n_args <= 13) {
      maxspeed_table = make_table(9, dur, SR);
      if (maxspeed_table == NULL)
         return die("JGRAN", "Either use the max. grain speed pfield (p13) "
                             "or make an old-style gen function in slot 9.");
   }
   if (n_args <= 14) {
      minintens_table = make_table(10, dur, SR);
      if (minintens_table == NULL)
         return die("JGRAN", "Either use the min. grain intensity pfield (p14) "
                             "or make an old-style gen function in slot 10.");
   }
   if (n_args <= 15) {
      maxintens_table = make_table(11, dur, SR);
      if (maxintens_table == NULL)
         return die("JGRAN", "Either use the max. grain intensity pfield (p15) "
                             "or make an old-style gen function in slot 11.");
   }
   if (n_args <= 16) {
      density_table = make_table(12, dur, SR);
      if (density_table == NULL)
         return die("JGRAN", "Either use the grain density pfield (p16) "
                             "or make an old-style gen function in slot 12.");
   }
   if (outputChannels() == 2) {
      if (n_args <= 17) {
         pan_table = make_table(13, dur, SR);
         if (pan_table == NULL)
            return die("JGRAN", "Either use the pan pfield (p17) or make an "
                                "old-style gen function in slot 13.");
      }
      if (n_args <= 18) {
         panvar_table = make_table(14, dur, SR);
         if (panvar_table == NULL)
            return die("JGRAN", "Either use the pan randomization pfield (p18) "
                               "or make an old-style gen function in slot 14.");
      }
   }

   // seed multipliers straight from Piche/Bezkorowajny source
   durnoi = new JGNoise((unsigned int) seed * 243);
   freqnoi = new JGNoise((unsigned int) seed * 734);
   pannoi = new JGNoise((unsigned int) seed * 634);
   ampnoi = new JGNoise((unsigned int) seed * 824);
   if (randomize_phase)
      phasenoi = new JGNoise((unsigned int) seed * 951);

   krate = resetval;
   skip = (int) (SR / (float) krate);

   return nSamps();
}


void JGRAN :: doupdate()
{
   double p[19];
   update(p, 19);

   amp = p[2];
   if (amp_table)
      amp *= amp_table->tick(currentFrame(), 1.0);

   // Get new random values.
   double randur = fabs(durnoi->tick());          // [0, 1]
   double ranfreq = fabs(freqnoi->tick());
   double ranamp = fabs(ampnoi->tick());
   if (randomize_phase)
      ranphase = fabs(phasenoi->tick()) * TWO_PI;

   // Read pfields or tables, depending on number of args to inst.  All this
   // nargs testing is for backwards compatibility with pre-v4 scores.

   double minfreq, maxfreq;
   if (nargs > 10)
      minfreq = p[10];
   else
      minfreq = minfreq_table->tick(currentFrame(), 1.0);
   if (nargs > 11)
      maxfreq = p[11];
   else
      maxfreq = maxfreq_table->tick(currentFrame(), 1.0);

   double minspeed, maxspeed;
   if (nargs > 12)
      minspeed = p[12];
   else
      minspeed = minspeed_table->tick(currentFrame(), 1.0);
   if (nargs > 13)
      maxspeed = p[13];
   else
      maxspeed = maxspeed_table->tick(currentFrame(), 1.0);

   double minintens, maxintens;
   if (nargs > 14)
      minintens = p[14];
   else
      minintens = minintens_table->tick(currentFrame(), 1.0);
   if (nargs > 15)
      maxintens = p[15];
   else
      maxintens = maxintens_table->tick(currentFrame(), 1.0);

   double density;
   if (nargs > 16)
      density = p[16];
   else
      density = density_table->tick(currentFrame(), 1.0);

   // Compute next grain duration.
   double logminspeed = log(minspeed);
   double speeddiff = fabs(log(maxspeed) - logminspeed);
   int tmp = (int) ((double) krate / exp(logminspeed + (randur * speeddiff)));
   next_graindur = ((double) tmp / (double) krate) + (1.0 / (double) krate);

   // Compute next grain amplitude multiplier.
   if (ranamp < density) {
      double intens = minintens + ((maxintens - minintens) * ranamp);
      next_grainamp = ampdb(intens);
   }
   else
      next_grainamp = 0.0;

   // Compute next carrier frequency.
   double logminfreq = log(minfreq);
   double freqdiff = fabs(log(maxfreq) - logminfreq);
   next_carfreq = exp(logminfreq + (ranfreq * freqdiff));

   // Compute next modulator frequency and depth, if we're doing FM.
   if (osctype == FM) {
      double modmult, index;
      if (nargs > 8)
         modmult = p[8];
      else
         modmult = modmult_table->tick(currentFrame(), 1.0);
      if (nargs > 9)
         index = p[9];
      else
         index = modindex_table->tick(currentFrame(), 1.0);

      next_modfreq = next_carfreq * modmult;
      next_moddepth = index * next_modfreq;
   }

   // Compute next pan.
   if (outputChannels() == 2) {
      double pan, panvar;
      if (nargs > 17)
         pan = p[17];
      else
         pan = pan_table->tick(currentFrame(), 1.0);
      if (nargs > 18)
         panvar = p[18];
      else
         panvar = panvar_table->tick(currentFrame(), 1.0);

      next_grainpan = pan;
      double ranpan = pannoi->tick();                     // [-1, 1]
      ranpan *= panvar;
      next_grainpan += ranpan;
      if (next_grainpan > 1.0)
         next_grainpan = 1.0;
      else if (next_grainpan < 0.0)
         next_grainpan = 0.0;
   }
}


int JGRAN :: run()
{
   for (int i = 0; i < framesToRun(); i++) {
      if (--branch <= 0) {                      // updates at control rate
         doupdate();
         branch = skip;
      }

      if (--gsampcount < 0) {                   // updates at each grain

         // set starting phase of carrier and grain envelope oscillators
         if (randomize_phase)
            car_oscil->setPhase(ranphase);
         grainenv_oscil->setPhase(0.0);

         // update these at start of each grain, not during one
         graindur = next_graindur;
         envoscil_freq = 1.0 / graindur;
         grainamp = next_grainamp;
         carfreq = next_carfreq;
         if (osctype == FM) {
            modfreq = next_modfreq;
            moddepth = next_moddepth;
         }
         if (outputChannels() == 2) {
            grainpan[0] = sqrt(next_grainpan);
            grainpan[1] = sqrt(1.0 - next_grainpan);
         }
         gsampcount = (int) (graindur * SR + 0.5);
         curgsamp = 0;
#ifdef DEBUG
         printf("dur=%f, amp=%f, cfrq=%f, mfrq=%f, mdep=%f, count=%d\n",
                 graindur, grainamp, carfreq, modfreq, moddepth, gsampcount);
#endif
      }

      double grainenv = grainenv_oscil->tick(envoscil_freq, grainamp);

      double carsig;
      if (osctype == FM) {
         double modsig = mod_oscil->tick(modfreq, moddepth);
         carsig = car_oscil->tick(carfreq + modsig, grainenv);
      }
      else
         carsig = car_oscil->tick(carfreq, grainenv);

      carsig *= amp;

      float out[2];
      if (outputChannels() == 2) {
         out[0] = carsig * grainpan[0];
         out[1] = carsig * grainpan[1];
      }
      else
         out[0] = carsig;

      rtaddout(out);
      increment();
      curgsamp++;
   }

   return framesToRun();
}


Instrument *makeJGRAN()
{
   JGRAN *inst;

   inst = new JGRAN();
   inst->set_bus_config("JGRAN");
   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("JGRAN", makeJGRAN);
}
#endif
