/* VOCODE2 - channel vocoder

   Performs a filter-bank analysis of the right input channel (the modulator),
   and uses the time-varying energy measured in the filter bands to control
   a corresponding filter bank that processes the left input channel (the
   carrier).  The two filter banks have identical characteristics, but there
   is a way to shift all of the center frequencies of the carrier's bank.

   p0  = output start time
   p1  = input start time (must be 0 for aux bus)
   p2  = duration
   p3  = amplitude multiplier (post-processing)

   Two ways to specify filter bank center frequencies:

    (1) Spread evenly above a given center frequency:
          p4 = number of filters (greater than 0)
          p5 = lowest filter center frequency (in Hz or oct.pc)
          p6 = center frequency spacing multiplier (greater than 1)
               (multiplies each cf by this to get next higher cf)

    (2) A list of center frequencies, given in function table 2 (Use gen 2.)
          p4 = 0 (must be zero: tells program to look for function table)
          p5 = transposition of function table, in oct.pc
          p6 = if > 1, add filters at p6 multiples of table frequencies
               E.g., if table has 300 and 500, and p6 is 2: 300, 500, 600, 1000
        Number of filters determined by length of function table.

   p7  = amount to transpose carrier filters (in Hz or oct.pc)
   p8  = filter bandwidth proportion of center frequency (greater than 0)
   p9  = filter response time (seconds)  [optional; default is 0.01]
         Determines how often changes in modulator power are measured.
   p10 = amount of high-passed modulator signal to mix with output
         (amplitude multiplier)  [optional; default is 0]
   p11 = cutoff frequency for high pass filter applied to modulator.
         This pfield ignored if p10 is zero.  [optional, default is 5000 Hz]
   p12 = amount of noise signal to mix into carrier before processing
         (amplitude multiplier applied to full-scale noise signal)
         [optional; default is 0]
   p13 = specifies how often (in samples) to get new random values from
         the noise generator.  This pfield is ignored if p12 is zero.
         [optional; default is 1 -- a new value every sample]  
   p14 = percent to left channel  [optional, default is 0.5]

   Assumes function table 1 is an amplitude curve for the note.  (Try gen 18.)
   Or you can just call setline.  If no setline or function table 1, uses a
   flat amplitude curve.  This curve, combined with the amplitude multiplier,
   affect the signal after processing by the filter bank.

   NOTES:

     -  Currently in RTcmix it's not possible for an instrument to take
        input from both an "in" bus and an "aux in" bus at the same time.
        So if you want the modulator to come from a microphone -- which
        must enter via an "in" bus -- and the carrier to come from a
        WAVETABLE instrument via an "aux" bus, then you must route the
        mic into the MIX instrument as a way to convert it from "in" to
        "aux in".  See "VOCODE2_1.sco" for an example.

     -  The "left" input channel comes from the bus with the lower number;
        the "right" input channel from the bus with the higher number.

     -  When specifying center frequencies...
           p6 = 2.0 will make a stack of octaves
           p6 = 1.5 will make a stack of perfect (Pythagorian) fifths
        Use this to get stacks of an equal tempered interval (in oct.pc):
           p6 = cpspch(interval) / cpspch(0.0)

     - More about p13:
       When this is greater than 1 sample, successive noise samples are
       connected using linear interpolation.  This acts as a low-pass filter;
       the higher the interval, the lower the cutoff frequency.


   John Gibson <johgibso at indiana dot edu>, 6/3/02.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "VOCODE2.h"
#include <rt.h>
#include <rtdefs.h>


VOCODE2 :: VOCODE2() : Instrument()
{
   branch = 0;
   in = NULL;
   noise = NULL;
   hipassmod = NULL;
}


VOCODE2 :: ~VOCODE2()
{
   delete [] in;
   for (int i = 0; i < numfilts; i++) {
      delete carrier_filt[i];
      delete modulator_filt[i];
      delete balancer[i];
   }
   delete noise;
   delete hipassmod;
}


int VOCODE2 :: init(float p[], int n_args)
{
   int   j, balance_window, subsample;
   float outskip, inskip, dur;
   float lowcf, spacemult, bwpct, responsetime, cf[MAXFILTS], carrier_transp;
   float hipasscf;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   numfilts = (int) p[4];
   lowcf = p[5];
   spacemult = p[6];
   carrier_transp = p[7];
   bwpct = p[8];
   responsetime = n_args > 9 ? p[9] : 0.01;        /* default: .01 secs */
   hipass_mod_amp = p[10];                         /* default: 0 */
   hipasscf = n_args > 11 ? p[11] : 5000.0;        /* default: 5000 Hz */
   noise_amp = p[12];                              /* default: 0 */
   subsample = (int) p[13];                        /* default: 1 (see below) */
   pctleft = n_args > 14 ? p[14] : 0.5;            /* default: center */

   nsamps = rtsetoutput(outskip, dur, this);
   rtsetinput(inskip, this);

   if (outputchans > 2)
      die("VOCODE2", "Output must be either mono or stereo.");
   if (inputchans != 2)
      die("VOCODE2",
      "Must use 2 input channels: 'left' for carrier; 'right' for modulator.");

   if (noise_amp > 0.0) {
      unsigned int seed = (unsigned int) 0;
      if (subsample < 1)
         subsample = 1;
      noise = new SubNoiseL(subsample, seed);
   }

   if (hipass_mod_amp > 0.0) {
      hipassmod = new Butter();
      hipassmod->setHighPass(hipasscf);
   }

   if (bwpct <= 0.0)
      die("VOCODE2", "Bandwidth proportion must be greater than 0.");

   balance_window = (int) (responsetime * SR);
   if (balance_window < 2) {
      warn("VOCODE2", "Response time too short ... changing to %.8f.",
                                                                     2.0 / SR);
      /* Otherwise, can get ear-splitting output. */
      balance_window = 2;
   }

   /* <numfilts> pfield lets user specify filter center frequencies by
      interval, in the form of a frequency multiplier, or by function table.
   */
   if (numfilts > 0) {   /* by interval */
      if (numfilts > MAXFILTS)
         die("VOCODE2", "Can only use %d filters.", MAXFILTS);
      if (spacemult <= 1.0)
         die("VOCODE2",
                     "Center frequency spacing factor must be greater than 1.");
      if (lowcf < 15.0)                        /* interpreted as oct.pc */
         lowcf = cpspch(lowcf);

      for (j = 0; j < numfilts; j++)
         cf[j] = lowcf * (float) pow((double) spacemult, (double) j);
   }
   else {                /* by function table */
      float transp = lowcf;                    /* pfield meaning changes */
      float *freqtable = floc(2);
      if (freqtable == NULL)
         die("VOCODE2",
                     "You haven't made the center freq. function (table 2).");
      numfilts = fsize(2);
      if (numfilts > MAXFILTS)
         die("VOCODE2", "Can only use %d filters.", MAXFILTS);
      advise("VOCODE2", "Reading center freqs from function table...");
      for (j = 0; j < numfilts; j++) {
         float freq = freqtable[j];
         if (freq < 15.0) {                    /* interpreted as oct.pc */
            if (transp)
               cf[j] = cpsoct(octpch(freq) + octpch(transp));
            else
               cf[j] = cpspch(freq);
         }
         else {                                /* interpreted as Hz */
            if (transp)
               cf[j] = cpsoct(octcps(freq) + octpch(transp));
            else
               cf[j] = freq;
         }
      }
      if (spacemult > 1.0) {
         int i = j;
         for (j = 0; i < MAXFILTS && j < numfilts; i++, j++)
            cf[i] = cf[j] * spacemult;
         numfilts = i;
      }
   }
   for (j = 0; j < numfilts; j++) {
      if (cf[j] > SR * 0.5) {
         warn("VOCODE2", "A cf was above Nyquist. Correcting...");
         cf[j] = SR * 0.5;
      }
   }

   advise("VOCODE2", "centerfreq  bandwidth");
   for (j = 0; j < numfilts; j++)
      advise(NULL, "         %10.4f %10.4f", cf[j], bwpct * cf[j]);

   if (carrier_transp)
      carrier_transp = octpch(carrier_transp);

   for (j = 0; j < numfilts; j++) {
      float thecf = cf[j];

      modulator_filt[j] = new Butter();
      modulator_filt[j]->setBandPass(thecf, bwpct * thecf);

      if (carrier_transp)
         thecf = cpsoct(octcps(thecf) + carrier_transp);

      carrier_filt[j] = new Butter();
      carrier_filt[j]->setBandPass(thecf, bwpct * thecf);

      balancer[j] = new Balance();
      balancer[j]->setWindowSize(balance_window);
      balancer[j]->setInitialGain(0.0);
   }

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(dur, lenamp, amptabs);
   }
   else
      advise("VOCODE2", "Setting phrase curve to all 1's.");

   skip = (int) (SR / (float) resetval);
   aamp = amp;                  /* in case amparray == NULL */

   return nsamps;
}


int VOCODE2 :: run()
{
   int   i, j, rsamps;
   float modsig, carsig;
   float out[MAXBUS];

   if (in == NULL)              /* first time, so allocate it */
      in = new float [RTBUFSAMPS * inputchans];

   Instrument :: run();

   rsamps = chunksamps * inputchans;
   rtgetin(in, this, rsamps);

   for (i = 0; i < rsamps; i += inputchans) {
      if (--branch < 0) {
         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs) * amp;
         branch = skip;
      }
      carsig = in[i];
      if (noise_amp > 0.0) {
         float noisig = noise->tick() * 32767.0;
         carsig += noisig * noise_amp;
      }
      modsig = in[i + 1];

      out[0] = 0.0;
      for (j = 0; j < numfilts; j++) {
         float mod = modulator_filt[j]->tick(modsig);
         float car = carrier_filt[j]->tick(carsig);
         CLAMP_DENORMALS(mod);
         CLAMP_DENORMALS(car);
         out[0] += balancer[j]->tick(car, mod);
      }
      if (hipass_mod_amp > 0.0) {
         float hpmodsig = hipassmod->tick(modsig);
         out[0] += hpmodsig * hipass_mod_amp;
      }

      out[0] *= aamp;
      if (outputchans == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      cursamp++;
   }

   return i;
}


Instrument *makeVOCODE2()
{
   VOCODE2 *inst;

   inst = new VOCODE2();
   inst->set_bus_config("VOCODE2");

   return inst;
}


void
rtprofile()
{
   RT_INTRO("VOCODE2", makeVOCODE2);
}


