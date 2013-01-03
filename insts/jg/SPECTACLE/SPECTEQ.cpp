/* SPECTEQ - FFT-based EQ

   p0  = output start time
   p1  = input start time
   p2  = input duration
   p3  = amplitude multiplier
   p4  = ring-down duration (can set to 0)
   p5  = FFT length (power of 2, usually 1024)
   p6  = window length (power of 2, usually FFT length * 2)
   p7  = window type (0: Hamming; see below for others)
   p8  = overlap - how much FFT windows overlap (any power of 2)
         1: no overlap, 2: hopsize=FFTlen/2, 4: hopsize=FFTlen/4, etc.
         2 or 4 is usually fine; 1 is fluttery; the higher the more CPU time
   p9  = input channel [optional, default is 0]
   p10 = percent to left channel [optional, default is .5]

   The following function tables control amplitude.

      Function table 1 is the input amplitude, spanning just the input
         duration.
      Function table 2 is the output amplitude, spanning the entire
         note, including ring-down duration.
      Function table 3 is the EQ table (i.e., amplitude scaling of each band),
         in dB (0 dB means no change, + dB boost, - dB cut).

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

   John Gibson <johgibso at indiana dot edu>, 8/12/03.
*/

//#define DUMP
//#define DEBUG
#include "SPECTEQ.h"

/* This inst is just a stripped down version of SPECTACLE.C.  -JGG */

/* --------------------------------------------------------------- SPECTEQ -- */
SPECTEQ :: SPECTEQ()
{
}


/* -------------------------------------------------------------- ~SPECTEQ -- */
SPECTEQ :: ~SPECTEQ()
{
   delete [] eqtable;
}


/* -------------------------------------------------------------- pre_init -- */
int SPECTEQ :: pre_init(double p[], int n_args)
{
   inchan = n_args > 9 ? (int) p[9] : 0;        /* default is chan 0 */
   pctleft = n_args > 10 ? p[10] : 0.5;         /* default is center */

   wetdry = 1.0;

   eqtable = floc(3);
   if (eqtable) {
      int len = fsize(3);
      eqtable = resample_functable(eqtable, len, half_fft_len);
   }
   else
      return die(instname(), "You haven't made the EQ function (table 3).");

   return 0;
}


/* ------------------------------------------------------------- post_init -- */
int SPECTEQ :: post_init(double p[], int n_args)
{
   return 0;
}


/* ---------------------------------------------------- dump_anal_channels -- */
void SPECTEQ :: dump_anal_channels()
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
void SPECTEQ :: modify_analysis()
{
   int   reading_input = (currentFrame() < input_end_frame);

   DPRINT1("modify_analysis: .............. reading_input=%d\n", reading_input);
#ifdef DUMP
   dump_anal_channels();
#endif

   for (int i = 0; i < half_fft_len; i++) {
      float mag;
      int index = i * 2;

      if (reading_input) {
         mag = anal_chans[index];
         mag *= ampdb(eqtable[i]);
      }
      else
         mag = 0.0;

      anal_chans[index] = mag;
   }
}


/* ----------------------------------------------------------- makeSPECTEQ -- */
Instrument *makeSPECTEQ()
{
   SPECTEQ *inst;

   inst = new SPECTEQ();
   inst->set_bus_config("SPECTEQ");

   return inst;
}

#ifndef MAXMSP
/* ------------------------------------------------------------- rtprofile -- */
void rtprofile()
{
   RT_INTRO("SPECTEQ", makeSPECTEQ);
}
#endif

