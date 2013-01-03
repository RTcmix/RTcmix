/* SPECTACLE - FFT-based delay processor

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

      Function table 3 is the EQ table (i.e., amplitude scaling of each band),
         in dB (0 dB means no change, + dB boost, - dB cut).
      Function table 4 is the delay time table.
      Function table 5 is the delay feedback table.  Values > 1 are dangerous!

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
#include "SPECTACLE.h"


/* ------------------------------------------------------------- SPECTACLE -- */
SPECTACLE :: SPECTACLE()
{
}


/* ------------------------------------------------------------ ~SPECTACLE -- */
SPECTACLE :: ~SPECTACLE()
{
   delete [] eqtable;
   delete [] deltimetable;
   delete [] feedbacktable;
   for (int i = 0; i < half_fft_len; i++) {
      if (mag_delay[i])
         delete mag_delay[i];
      if (phase_delay[i])
         delete phase_delay[i];
   }
}


/* -------------------------------------------------------------- pre_init -- */
int SPECTACLE :: pre_init(double p[], int n_args)
{
   wetdry = n_args > 9 ? p[9] : 1.0;            /* default is 1 */
   inchan = n_args > 10 ? (int) p[10] : 0;      /* default is chan 0 */
   pctleft = n_args > 11 ? p[11] : 0.5;         /* default is center */

   if (wetdry < 0.0 || wetdry > 1.0)
      return die(instname(), "Wet/dry must be between 0 and 1.");

   eqtable = floc(3);
   if (eqtable) {
      int len = fsize(3);
      eqtable = resample_functable(eqtable, len, half_fft_len);
   }
   else
      return die(instname(), "You haven't made the EQ function (table 3).");

   deltimetable = floc(4);
   if (deltimetable) {
      int len = fsize(4);
      deltimetable = resample_functable(deltimetable, len, half_fft_len);
   }
   else
      return die(instname(),
                 "You haven't made the delay time function (table 4).");

   /* Compute maximum delay lag and create delay lines for FFT magnitude
      and phase values.  Make ringdur at least as long as the longest
      delay time.  Remember that these delays function at the decimation
      rate, not at audio rate.
   */
   long maxdelsamps = (long) (MAXDELTIME * SR / decimation + 0.5);
   float maxtime = 0.0;
   for (int i = 0; i < half_fft_len; i++) {
      float deltime = deltimetable[i];
      if (deltime < 0.0 || deltime > MAXDELTIME)
         return die(instname(), "Delay times must be >= 0 and <= %g.",
                                                                  MAXDELTIME);
      float samps = deltime * SR / (float) decimation;
      assert(samps <= maxdelsamps);
      mag_delay[i] = new DLineN(maxdelsamps);
      phase_delay[i] = new DLineN(maxdelsamps);
      if (deltime > maxtime)
         maxtime = deltime;
//    DPRINT2("%d: %g\n", i, samps);
   }
   if (ringdur < maxtime)
      ringdur = maxtime;   /* but will still cut off any trailing feedback */

   DPRINT3("decimation=%d, maxdelsamps=%ld, ringdur=%g\n",
                                 decimation, maxdelsamps, ringdur);

   feedbacktable = floc(5);
   if (feedbacktable) {
      int len = fsize(5);
      feedbacktable = resample_functable(feedbacktable, len, half_fft_len);
   }
   else
      return die(instname(),
                  "You haven't made the delay feedback function (table 5).");

   return 0;
}


/* ------------------------------------------------------------- post_init -- */
int SPECTACLE :: post_init(double p[], int n_args)
{
   return 0;
}


/* ---------------------------------------------------- dump_anal_channels -- */
void SPECTACLE :: dump_anal_channels()
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
void SPECTACLE :: modify_analysis()
{
   int   reading_input = (currentFrame() < input_end_frame);

   DPRINT1("modify_analysis: .............. reading_input=%d\n", reading_input);
#ifdef DUMP
   dump_anal_channels();
#endif

   for (int i = 0; i < half_fft_len; i++) {
      float mag, phase;
      int index = i * 2;

      if (reading_input) {
         mag = anal_chans[index];
         phase = anal_chans[index + 1];
         mag *= ampdb(eqtable[i]);
      }
      else {
         mag = 0.0;
         phase = anal_chans[index + 1];
      }

      float deltime = deltimetable[i];
      if (deltime == 0.0) {
         anal_chans[index] = mag;
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

         float newmag = mag_delay[i]->getSample(delsamps);
         float newphase = phase_delay[i]->getSample(delsamps);
         anal_chans[index] = newmag;
         anal_chans[index + 1] = newphase;
         mag_delay[i]->putSample(mag + (newmag * feedbacktable[i]));

         if (reading_input)
            phase_delay[i]->putSample(phase);
         else
            phase_delay[i]->putSample(newphase);
      }
   }
}


/* --------------------------------------------------------- makeSPECTACLE -- */
Instrument *makeSPECTACLE()
{
   SPECTACLE *inst;

   inst = new SPECTACLE();
   inst->set_bus_config("SPECTACLE");

   return inst;
}

#ifndef MAXMSP
/* ------------------------------------------------------------- rtprofile -- */
void rtprofile()
{
   RT_INTRO("SPECTACLE", makeSPECTACLE);
}
#endif

