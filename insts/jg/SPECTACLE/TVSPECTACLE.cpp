/* TVSPECTACLE - FFT-based delay processor with time-varying properties

   p0  = output start time
   p1  = input start time
   p2  = input duration
   p3  = amplitude multiplier
   p4  = ring-down duration
   p5  = FFT length (power of 2, usually 1024)
   p6  = window length (power of 2, usually FFT length * 2)
   p7  = window type (0: Hamming; see below for others)
   p8  = overlap - how much FFT windows overlap (any power of 2)
         1: no overlap, 2: hopsize=FFTlen/2, 4: hopsize=FFTlen/4, etc.
         2 or 4 is usually fine; 1 is fluttery; the higher the more CPU time
   p9  = wet/dry mix (0: dry -> 1: wet) [optional, default is 1]
   p10 = input channel [optional, default is 0]
   p11 = percent to left channel [optional, default is .5]

   The following function tables control amplitude.

      Function table 1 is the input amplitude, spanning just the input
         duration.
      Function table 2 is the output amplitude, spanning the entire
         note, including ring-down duration.

   The following function tables control EQ, delay time, and delay feedback
   for all the frequency bands of the FFT.  Think of them as curves on a
   graph with frequency on the x axis and amplitude, delay time or feedback
   on the y axis.

      Function table 3 is EQ table A (i.e., amplitude scaling of each band),
         in dB (0 dB means no change, + dB boost, - dB cut).
      Function table 4 is delay time table A.
      Function table 5 is delay feedback table A.  Values > 1 are dangerous!
      Function table 6 is EQ table B.
      Function table 7 is delay time table B.
      Function table 8 is delay feedback table B.

   These tables control the movement between the A and B tables described
   above.  The values should be in the range [0, 1]; if not, the instrument
   prints a warning and pins the values to that range.  A value of 0 means
   to read only from table A; a value of 1, from table B; and values in
   between 0 and 1 request linear interpolation between the two tables.

      Function table 9 describes the curve between EQ tables A and B.
      Function table 10 describes the curve between delay time tables A and B.
      Function table 11 describes the curve between delay feedback tables A
         and B.

   NOTES:

      p7 - window type:
            0: Hamming, 1: Hanning, 2: Rectangle, 3: Triangle, 4: Blackman,
            5: Kaiser
           When in doubt, use Hamming.

      p8 - overlap:
            1: no overlap, 2: hopsize=FFTlen/2, 4: hopsize=FFTlen/4, etc.
            2 or 4 is usually fine; 1 is fluttery; higher overlaps use more CPU.
            Also possible to use negative powers of 2, e.g., .5, .25, .125, etc.
            This leaves a gap between successive FFTs, creating ugly robotic
            effects -- beware of clipping.

      p9 - wet/dry mix:
            This is pre-EQ.

   John Gibson <johgibso at indiana dot edu>, 11/28/02.
*/

//#define DUMP
//#define DEBUG
#include "TVSPECTACLE.h"


/* ----------------------------------------------------------- TVSPECTACLE -- */
TVSPECTACLE :: TVSPECTACLE()
{
}


/* ---------------------------------------------------------- ~TVSPECTACLE -- */
TVSPECTACLE :: ~TVSPECTACLE()
{
   delete [] eqtableA;
   delete [] deltimetableA;
   delete [] feedbacktableA;
   delete [] eqtableB;
   delete [] deltimetableB;
   delete [] feedbacktableB;
   for (int i = 0; i < half_fft_len; i++) {
      if (mag_delay[i])
         delete mag_delay[i];
      if (phase_delay[i])
         delete phase_delay[i];
   }
}


/* ----------------------------------------------------------- check_curve -- */
static int check_curve(double table[], int len, double min, double max)
{
   int change = 0;
   for (int i = 0; i < len; i++) {
      if (table[i] < min) {
         table[i] = min;
         change = 1;
      }
      else if (table[i] > max) {
         table[i] = max;
         change = 1;
      }
   }
   return change;
}


/* -------------------------------------------------------------- pre_init -- */
int TVSPECTACLE :: pre_init(double p[], int n_args)
{
   wetdry = n_args > 9 ? p[9] : 1.0;            /* default is 1 */
   inchan = n_args > 10 ? (int) p[10] : 0;      /* default is chan 0 */
   pctleft = n_args > 11 ? p[11] : 0.5;         /* default is center */

   if (wetdry < 0.0 || wetdry > 1.0)
      return die(instname(), "Wet/dry must be between 0 and 1.");

   eqtableA = floc(3);
   if (eqtableA) {
      int len = fsize(3);
      eqtableA = resample_functable(eqtableA, len, half_fft_len);
   }
   else
      return die(instname(), "You haven't made EQ function A (table 3).");

   eqtableB = floc(6);
   if (eqtableB) {
      int len = fsize(6);
      eqtableB = resample_functable(eqtableB, len, half_fft_len);
   }
   else
      return die(instname(), "You haven't made EQ function B (table 6).");

   deltimetableA = floc(4);
   if (deltimetableA) {
      int len = fsize(4);
      deltimetableA = resample_functable(deltimetableA, len, half_fft_len);
   }
   else
      return die(instname(),
                        "You haven't made delay time function A (table 4).");

   deltimetableB = floc(7);
   if (deltimetableB) {
      int len = fsize(7);
      deltimetableB = resample_functable(deltimetableB, len, half_fft_len);
   }
   else
      return die(instname(),
                        "You haven't made delay time function B (table 7).");

   /* Compute maximum delay lag and create delay lines for FFT magnitude
      and phase values.  Make ringdur at least as long as the longest
      delay time.  Remember that these delays function at the decimation
      rate, not at audio rate.
   */
   maxdelsamps = (long) (MAXDELTIME * SR / decimation + 0.5);
   float maxtime = 0.0;
   for (int i = 0; i < half_fft_len; i++) {

      /* Check delay time table A. */
      float deltime = deltimetableA[i];
      if (deltime < 0.0 || deltime > MAXDELTIME)
         return die(instname(),
                    "Delay times must be >= 0 and <= %g. (The value in "
                    "table A at %d is %g.)", MAXDELTIME, i, deltime);
      float samps = deltime * SR / (float) decimation;
      assert(samps <= maxdelsamps);
      if (deltime > maxtime)
         maxtime = deltime;

      /* Check delay time table B. */
      deltime = deltimetableB[i];
      if (deltime < 0.0 || deltime > MAXDELTIME)
         return die(instname(),
                    "Delay times must be >= 0 and <= %g. (The value in "
                    "table B at %d is %g.)", MAXDELTIME, i, deltime);
      samps = deltime * SR / (float) decimation;
      assert(samps <= maxdelsamps);
      if (deltime > maxtime)
         maxtime = deltime;

      mag_delay[i] = new DLineN(maxdelsamps);
      phase_delay[i] = new DLineN(maxdelsamps);
   }
   if (ringdur < maxtime)
      ringdur = maxtime;   /* but will still cut off any trailing feedback */

   DPRINT3("decimation=%d, maxdelsamps=%ld, ringdur=%g\n",
                                 decimation, maxdelsamps, ringdur);

   feedbacktableA = floc(5);
   if (feedbacktableA) {
      int len = fsize(5);
      feedbacktableA = resample_functable(feedbacktableA, len, half_fft_len);
   }
   else
      return die(instname(),
                     "You haven't made delay feedback function A (table 5).");

   feedbacktableB = floc(8);
   if (feedbacktableB) {
      int len = fsize(8);
      feedbacktableB = resample_functable(feedbacktableB, len, half_fft_len);
   }
   else
      return die(instname(),
                     "You haven't made delay feedback function B (table 8).");

   eqcurve = floc(9);
   if (eqcurve) {
      int len = fsize(9);
      if (check_curve(eqcurve, len, 0.0, 1.0))
         rtcmix_warn(instname(), "EQ curve values must be between 0 and 1.\n"
                          "Fixing...");
      tableset(SR, inputdur + ringdur, len, eqcurvetabs);
      eq_curve_weight = eqcurve[0];
   }
   else {
      rtcmix_advise(instname(), "Setting EQ table curve to all 0's (no table 9).");
      eq_curve_weight = 0.0;
   }

   deltimecurve = floc(10);
   if (deltimecurve) {
      int len = fsize(10);
      if (check_curve(deltimecurve, len, 0.0, 1.0))
         rtcmix_warn(instname(), "Delay time curve values must be between 0 and 1.\n"
                          "Fixing...");
      tableset(SR, inputdur + ringdur, len, deltimecurvetabs);
      deltime_curve_weight = deltimecurve[0];
   }
   else {
      rtcmix_advise(instname(), "Setting EQ table curve to all 0's (no table 10).");
      deltime_curve_weight = 0.0;
   }

   feedbackcurve = floc(11);
   if (feedbackcurve) {
      int len = fsize(11);
      if (check_curve(feedbackcurve, len, 0.0, 1.0))
         rtcmix_warn(instname(), "Feedback curve values must be between 0 and 1.\n"
                          "Fixing...");
      tableset(SR, inputdur + ringdur, len, feedbackcurvetabs);
      feedback_curve_weight = feedbackcurve[0];
   }
   else {
      rtcmix_advise(instname(), "Setting EQ table curve to all 0's (no table 11).");
      feedback_curve_weight = 0.0;
   }

//printf("deltimecurvetabs: %f, %f\n", deltimecurvetabs[0], deltimecurvetabs[1]);
   return 0;
}


/* ------------------------------------------------------------- post_init -- */
int TVSPECTACLE :: post_init(double p[], int n_args)
{
   return 0;
}


/* ---------------------------------------------------- dump_anal_channels -- */
void TVSPECTACLE :: dump_anal_channels()
{
#ifdef DUMP
   printf("\n----------------------------------------\n");
   for (int i = 0; i < half_fft_len; i++)
      int index = i * 2;
      printf("%3d [%.3f Hz]:\t%f Hz, %f\n",
               i,
               i * fund_anal_freq,
               anal_chans[index + 1],
               anal_chans[index]);
#endif
}


/* ------------------------------------------------------- modify_analysis -- */
void TVSPECTACLE :: modify_analysis()
{
   int   reading_input = (currentFrame() < input_end_frame);

   DPRINT1("modify_analysis: .............. reading_input=%d\n", reading_input);
#ifdef DUMP
   dump_anal_channels();
#endif

   if (currentFrame() >= latency) {
      if (eqcurve)
         eq_curve_weight = tablei(currentFrame() - latency, eqcurve,
                                                         eqcurvetabs);
      if (deltimecurve)
         deltime_curve_weight = tablei(currentFrame() - latency, deltimecurve,
                                                         deltimecurvetabs);
      if (feedbackcurve)
         feedback_curve_weight = tablei(currentFrame() - latency, feedbackcurve,
                                                         feedbackcurvetabs);
   }
#ifdef DEBUG
   printf("weights: eq=%f, del=%f, fb=%f (cursamp=%d, latency=%d)\n",
            eq_curve_weight, deltime_curve_weight, feedback_curve_weight,
            currentFrame(), latency);
#endif

   for (int i = 0; i < half_fft_len; i++) {
      float mag, phase;
      int index = i * 2;

      if (reading_input) {
         mag = anal_chans[index];
         phase = anal_chans[index + 1];
      }
      else {
         mag = 0.0;
         phase = anal_chans[index + 1];
      }

      float eq = ((1.0 - eq_curve_weight) * eqtableA[i])
                                          + (eq_curve_weight * eqtableB[i]);
      float deltime = ((1.0 - deltime_curve_weight) * deltimetableA[i])
                                 + (deltime_curve_weight * deltimetableB[i]);

      if (deltime == 0.0) {
         anal_chans[index] = mag * ampdb(eq);
         anal_chans[index + 1] = phase;
      }
      else {
         long delsamps = (long)(deltime * SR + 0.5) / decimation;

         /* Not sure why this is necessary, but without it, delayed copies
            sound distorted.
         */
         if (int_overlap > 1) {
            int remainder = delsamps % int_overlap;
            if (remainder)
               delsamps -= remainder;
         }
         assert(delsamps >= 0 && delsamps <= maxdelsamps);

         float newmag = mag_delay[i]->getSample(delsamps);
         float newphase = phase_delay[i]->getSample(delsamps);
         anal_chans[index] = newmag * ampdb(eq);
         anal_chans[index + 1] = newphase;

         float feedback = ((1.0 - feedback_curve_weight) * feedbacktableA[i])
                                 + (feedback_curve_weight * feedbacktableB[i]);
         mag_delay[i]->putSample(mag + (newmag * feedback));

         if (reading_input)
            phase_delay[i]->putSample(phase);
         else
            phase_delay[i]->putSample(newphase);
      }
   }
}


/* ------------------------------------------------------- makeTVSPECTACLE -- */
Instrument *makeTVSPECTACLE()
{
   TVSPECTACLE *inst;

   inst = new TVSPECTACLE();
   inst->set_bus_config("TVSPECTACLE");

   return inst;
}

#ifndef MAXMSP
/* ------------------------------------------------------------- rtprofile -- */
void rtprofile()
{
   RT_INTRO("TVSPECTACLE", makeTVSPECTACLE);
}
#endif

