/* JGRAN - granular synthesis with FM or AS grains

   This was derived from a Cecilia module (StochasticGrains) by
   Mathieu Bezkorowajny and Jean Piche. See also Mara Helmuth's
   sgran (RTcmix/insts.std) for more control over randomness.

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = random seed (any integer; if 0, seed from system clock) [default: 0]
   p4 = oscillator configuration (0: additive, 1: FM, 2: sampled) [default: 0]
   p5 = randomize oscillator starting phase (0: no, 1: yes) [default: yes]

   Function table (makegen) assignments:

    1   overall amplitude envelope (or use setline)
    2   grain envelope
    3   grain waveform
    4   modulator frequency multiplier            (can skip if p4 is not 1)
    5   index of modulation envelope (per grain)  (can skip if p4 is not 1)
    6   minimum grain frequency
    7   maximum grain frequency
    8   minumum grain speed
    9   maximum grain speed
   10   minumum grain intensity
   11   maximum grain intensity
   12   grain density
   13   grain stereo location                     (can skip if output is mono)
   14   grain stereo location randomization       (can skip if output is mono)

   NOTES:
     1. Produces only one stream of non-overlapping grains. To get more
        streams, call JGRAN more than once (maybe with different seeds).
     2. Uses non-interpolating oscillators for efficiency, so make large
        tables for the grain waveform and grain envelope.
     3. For functions 5-11, either use gen18 or make the slot number negative,
        to tell the gen routine not to rescale values to fit between 0 and 1.
     4. Grains within one stream (note) never overlap. (Use sgran for this.)
     5. Ability to use a sampled waveform is not implemented yet.

   John Gibson (johngibson@virginia.edu), 4/15/00.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
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
make_table(int function_num, float dur)
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
   int   seed, wavetablen = DEFAULT_WAVETABLE_SIZE;
   float outskip, dur;

   outskip = p[0];
   dur = p[1];
   amp = p[2];
   seed = (int) p[3];
   osctype = (OscType) p[4];
   randomize_phase = n_args > 5 ? (int) p[5] : 1;           /* default: yes */

   if (outputchans > 2)
      return die("JGRAN", "Output must be mono or stereo.");

   nsamps = rtsetoutput(outskip, dur, this);

   amp_table = make_table(1, dur);
   if (amp_table == NULL) {
      advise("JGRAN", "Setting phrase curve to all 1's.");
      aamp = amp;
   }

   double *envtab = floc(2);
   if (envtab) {
      int len = fsize(2);
      grainenv_oscil = new OscilN(SR, 0.0, envtab, len);
   }
   else
      return die("JGRAN",
                 "You haven't made the grain envelope function (table 2).");

   double *wavetab = floc(3);
   if (wavetab)
      wavetablen = fsize(3);
   else
      advise("JGRAN", "Using sine for grain waveform (no table 3).");

   if (osctype == FM) {
      modmult_table = make_table(4, dur);
      if (modmult_table == NULL)
         return die("JGRAN", "You haven't made the modulation frequency "
                             "multiplier function (table 4).");
      modindex_table = make_table(5, dur);
      if (modindex_table == NULL)
         return die("JGRAN",
                    "You haven't made the index envelope function (table 5).");
   }
   minfreq_table = make_table(6, dur);
   if (minfreq_table == NULL)
      return die("JGRAN", "You haven't made the minimum grain frequency "
                          "function (table 6).");
   maxfreq_table = make_table(7, dur);
   if (maxfreq_table == NULL)
      return die("JGRAN", "You haven't made the maximum grain frequency "
                          "function (table 7).");
   minspeed_table = make_table(8, dur);
   if (minspeed_table == NULL)
      return die("JGRAN", "You haven't made the minimum grain speed "
                          "function (table 8).");
   maxspeed_table = make_table(9, dur);
   if (maxspeed_table == NULL)
      return die("JGRAN", "You haven't made the maximum grain speed "
                          "function (table 9).");
   minintens_table = make_table(10, dur);
   if (minintens_table == NULL)
      return die("JGRAN", "You haven't made the minimum grain intensity "
                          "function (table 10).");
   maxintens_table = make_table(11, dur);
   if (maxintens_table == NULL)
      return die("JGRAN", "You haven't made the maximum grain intensity "
                          "function (table 11).");
   density_table = make_table(12, dur);
   if (density_table == NULL)
      return die("JGRAN", "You haven't made the density function (table 12).");
   if (outputchans == 2) {
      pan_table = make_table(13, dur);
      if (pan_table == NULL)
         return die("JGRAN", "You haven't made the stereo location "
                             "function (table 13).");
      panvar_table = make_table(14, dur);
      if (panvar_table == NULL)
         return die("JGRAN", "You haven't made the stereo location "
                             "randomization function (table 14).");
   }

   car_oscil = new OscilN(SR, 0.0, wavetab, wavetablen);
   if (osctype == FM)
      mod_oscil = new OscilN(SR, 0.0, wavetab, wavetablen);

   durnoi = new Noise((unsigned int) seed * 243);
   freqnoi = new Noise((unsigned int) seed * 734);
   pannoi = new Noise((unsigned int) seed * 634);
   ampnoi = new Noise((unsigned int) seed * 824);
   if (randomize_phase)
      phasenoi = new Noise((unsigned int) seed * 951);

   krate = resetval;
   skip = (int) (SR / (float) krate);

   return nsamps;
}


int JGRAN :: run()
{
   int      i;
   float    carsig, grainenv, out[2];

   for (i = 0; i < chunksamps; i++) {
      if (--branch < 0) {                       /* updates at control rate */
         int    tmp;
         float  minfreq, maxfreq, freqdiff, minspeed, maxspeed, speeddiff;
         float  ranamp, ranpan, density, minintens, maxintens;
         double randur, ranfreq, logminfreq, logminspeed;

         if (amp_table)
            aamp = amp_table->tick(cursamp, amp);

         /* get new random values */
         randur = fabs((double) durnoi->tick());          /* [0, 1] */
         ranfreq = fabs((double) freqnoi->tick());
         ranamp = (float) fabs((double) ampnoi->tick());
         if (randomize_phase)
            ranphase = (float) fabs((double) phasenoi->tick()) * TWO_PI;

         /* read tables */
         minfreq = minfreq_table->tick(cursamp, 1.0);
         maxfreq = maxfreq_table->tick(cursamp, 1.0);
         minspeed = minspeed_table->tick(cursamp, 1.0);
         maxspeed = maxspeed_table->tick(cursamp, 1.0);
         minintens = minintens_table->tick(cursamp, 1.0);
         maxintens = maxintens_table->tick(cursamp, 1.0);
         density = density_table->tick(cursamp, 1.0);

         /* compute next grain duration */
         logminspeed = log((double) minspeed);
         speeddiff = fabs(log((double) maxspeed) - logminspeed);
         tmp = (int) ((double) krate / exp(logminspeed + (randur * speeddiff)));
         next_graindur = ((float) tmp / (float) krate) + (1.0 / (float) krate);

         /* compute next grain amplitude multiplier */
         if (ranamp < density) {
            float intens = minintens + ((maxintens - minintens) * ranamp);
            next_grainamp = (float) ampdb(intens);
         }
         else
            next_grainamp = 0.0;

         /* compute next carrier frequency */
         logminfreq = log((double) minfreq);
         freqdiff = fabs(log((double) maxfreq) - logminfreq);
         next_carfreq = (float) exp(logminfreq + (ranfreq * freqdiff));

         /* compute next modulator frequency and depth, if we're doing FM */
         if (osctype == FM) {
            float modmult = modmult_table->tick(cursamp, 1.0);
            float index = modindex_table->tick(cursamp, 1.0);
            next_modfreq = next_carfreq * modmult;
            next_moddepth = index * next_modfreq;
         }

         /* compute next pctleft */
         if (outputchans == 2) {
            float panvar = panvar_table->tick(cursamp, 1.0);
            next_grainpan = pan_table->tick(cursamp, 1.0);
            ranpan = pannoi->tick();                      /* [-1, 1] */
            ranpan *= panvar;
            next_grainpan += ranpan;
            if (next_grainpan > 1.0)
               next_grainpan = 1.0;
            else if (next_grainpan < 0.0)
               next_grainpan = 0.0;
         }
         branch = skip;
      }

      if (--gsampcount < 0) {                   /* updates at each grain */

         /* set starting phase of carrier and grain envelope oscillators */
         if (randomize_phase)
            car_oscil->setPhase(ranphase);
         grainenv_oscil->setPhase(0.0);

         /* update these at start of each grain, not during one */
         graindur = next_graindur;
         envoscil_freq = 1.0 / graindur;
         grainamp = next_grainamp;
         carfreq = next_carfreq;
         if (osctype == FM) {
            modfreq = next_modfreq;
            moddepth = next_moddepth;
         }
         if (outputchans == 2) {
            grainpan[0] = (float) sqrt((double) next_grainpan);
            grainpan[1] = (float) sqrt(1.0 - (double) next_grainpan);
         }
         gsampcount = (int) (graindur * SR + 0.5);
         curgsamp = 0;
#ifdef DEBUG
         printf("dur=%f, amp=%f, cfrq=%f, mfrq=%f, mdep=%f, count=%d\n",
                 graindur, grainamp, carfreq, modfreq, moddepth, gsampcount);
#endif
      }

      grainenv = grainenv_oscil->tick(envoscil_freq, grainamp);

      if (osctype == FM) {
         float modsig = mod_oscil->tick(modfreq, moddepth);
         carsig = car_oscil->tick(carfreq + modsig, grainenv);
      }
      else
         carsig = car_oscil->tick(carfreq, grainenv);

      carsig *= aamp;                          /* apply overall setline */

      if (outputchans == 2) {
         out[0] = carsig * grainpan[0];
         out[1] = carsig * grainpan[1];
      }
      else
         out[0] = carsig;

      rtaddout(out);
      cursamp++;
      curgsamp++;
   }

   return i;
}


Instrument *makeJGRAN()
{
   JGRAN *inst;

   inst = new JGRAN();
   inst->set_bus_config("JGRAN");
   return inst;
}

void
rtprofile()
{
   RT_INTRO("JGRAN", makeJGRAN);
}


