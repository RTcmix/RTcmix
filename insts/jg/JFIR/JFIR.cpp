/* JFIR - FIR filter, designed from a frequency response curve

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier *
   p4 = filter order (higher order allows steeper slope)
   p5 = input channel [optional, default is 0]
   p6 = pan (in percent-to-left form: 0-1) [optional; default is 0.5]
   p7 = bypass filter (0: no, 1: yes) [optional, default is 0]
   p8 = reference to frequency response table [optional; if missing, must
        use gen 2 **]

   p3 (amplitude), p6 (pan) and p7 (bypass) can receive dynamic updates from
   a table or real-time control source.

   The desired frequency response curve is described by a table specification
   of <frequency, amplitude> pairs.  Frequency is in Hz, from 0 to Nyquist;
   amp is from 0 to 1.  Ideally, frequencies with amplitude of 1 are passed
   without attenuation; those with amplitude of 0 are attenuated totally.  But
   this behavior depends on the order of the filter. Try an order of 200, and
   increase that as needed.

   Example:

      nyq = 44100 / 2
      table = maketable("line", 5000, 0,0, 200,0, 300,1, 2000,1, 4000,0, nyq,0)

   With a high order, this should attenuate everything below 200 Hz and
   above 4000 Hz.

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p3 (amplitude), even if the latter is dynamic.

   ** If p8 is missing, you must use an old-style gen table 2 for the
   frequency response curve.


   John Gibson (jgg9c@virginia.edu), 7/3/99; rev for v4, JGG, 7/24/04
   Filter design code adapted from Bill Schottstaedt's Snd.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include "JFIR.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG
//#define PRINT_RESPONSE
#define NCOLUMNS 160      // 1 data point per terminal column
#define NROWS    60


JFIR :: JFIR() : Instrument()
{
   in = NULL;
   branch = 0;
}


JFIR :: ~JFIR()
{
   delete [] in;
   delete filt;
}


int JFIR :: init(double p[], int n_args)
{
   nargs = n_args;
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   int order = (int) p[4];
   inchan = (int) p[5];

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   insamps = (int) (dur * SR + 0.5);
   if (inchan >= inputChannels())
      return die("JFIR", "You asked for channel %d of a %d-channel file.",
                                                      inchan, inputChannels());
   float ringdur = (float) order / SR;
   if (rtsetoutput(outskip, dur + ringdur, this) == -1)
      return DONT_SCHEDULE;

   double *response_table = NULL;
	int tablelen = 0;
	if (n_args > 8) {      // handle table coming in as optional p8 TablePField
		response_table = (double *) getPFieldTable(8, &tablelen);
	}
	if (response_table == NULL) {
		response_table = floc(2);
		if (response_table == NULL)
			return die("JFIR", "Either use the frequency response pfield (p8) "
                    "or make an old-style gen function in slot 2.");
		tablelen = fsize(2);
	}

   if (order < 1)
      return die("JFIR", "Order must be greater than 0.");

   filt = new NZero(SR, order);
   filt->designFromFunctionTable(response_table, tablelen, 0, 0);
#ifdef PRINT_RESPONSE
   print_freq_response();
#endif

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(SR, dur, lenamp, amptabs);
   }

   return nSamps();
}


void JFIR :: doupdate()
{
   amp = update(3, insamps);
   if (amparray)
      amp *= tablei(currentFrame(), amparray, amptabs);

   if (nargs > 6)
      pctleft = update(6);
   else
      pctleft = 0.5;            // default is center

   if (nargs > 7)
      bypass = bool(update(7));
   else
      bypass = false;           // default is no
}


int JFIR :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int JFIR :: run()
{
   const int samps = framesToRun() * inputChannels();

   rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         doupdate();
         branch = getSkip();
      }

      float insig;
      if (currentFrame() < insamps)          // still taking input
         insig = in[i + inchan] * amp;
      else                                   // in ring-down phase
         insig = 0.0;

      float out[2];
      if (bypass)
         out[0] = insig;
      else
         out[0] = filt->tick(insig);

      if (outputchans == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


void JFIR :: print_freq_response()
{
   float nyquist = SR * 0.5;
   float incr = nyquist / (float)NCOLUMNS;
   float *a, amps[NCOLUMNS], maxamp, rowamp;

   a = amps;
   for (float freq = 0.0; freq < nyquist; freq += incr)
      *a++ = filt->getFrequencyResponse(freq / nyquist);

   maxamp = 1.2;
   rowamp = maxamp / (float)NROWS;
   for (int row = NROWS; row > 0; row--) {
      maxamp -= rowamp;
      for (int col = 0; col < NCOLUMNS; col++) {
         if (amps[col] >= maxamp) {
            printf(".");
            amps[col] = -1.0;       // skip it from now on
         }
         else
            printf(" ");
      }
      printf("\n");
   }

#ifdef DEBUG
   printf("frequency response:\n");
   printf("freq: %8.2f   amp: %16.10f\n", freq, a);
#endif
}


Instrument *makeJFIR()
{
   JFIR *inst;

   inst = new JFIR();
   inst->set_bus_config("JFIR");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("JFIR", makeJFIR);
}
#endif

