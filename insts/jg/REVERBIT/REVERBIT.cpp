/* REVERBIT: Lansky's fast reverberator
 *
 * Parameters:
 *    p0 = output start time
 *    p1 = input start time
 *    p2 = input duration
 *    p3 = amplitude multiplier (of input signal)
 *    p4 = reverb time (must be greater than 0)
 *    p5 = reverb percent (between 0 and 1 inclusive)
 *    p6 = right channel delay time (must be greater than 0)
 *    p7 = cutoff freq for low-pass filter (in cps)  (0 to disable filter)
 *    p8 = apply DC blocking filter (0: No, 1: Yes) [optional, default is Yes]
 *         (set to zero for a bit more speed)
 *
 *    Assumes function slot 1 is the amplitude envelope.
 *    Or you can just call setline. If no setline or function table 1, uses
 *    flat amplitude curve (all 1's). This curve, and p3, affect the
 *    input signal, not the output signal.
 *
 *    Input file can be mono or stereo. Output file must be stereo.
 *
 *    To quote Lansky, this is meant to "put a gloss on a signal."
 *    Here's how it works:
 *      (1) Runs the input signal into a simple Schroeder reverberator,
 *          scaling the output of that by the reverb percent and flipping
 *          its phase.
 *      (2) Puts output of (1) into a delay line, length determined by p6.
 *      (3) Adds output of (1) to dry signal, and places in left channel.
 *      (4) Adds output of delay to dry signal, and places in right channel.
 *
 *    I added the optional low-pass filter to the output of (1) and the
 *    optional DC blocking filter.
 *
 *    RT'd and spruced up a bit by John Gibson (jgg9c@virginia.edu), 6/24/99
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "REVERBIT.h"
#include <rt.h>
#include <rtdefs.h>

#define DELAY_FACTOR   2       /* Determines length of delay line */
#define MIN_DELAY      .10     /* Uses extra-large delay line below this */
#define RVTSLOP        0.2     /* Extra ring-down time */

/* The DC-blocking code is from Perry Cook's STK (STK98v2/DCBlock.cpp).
   It works pretty well (and quickly) for all but extreme cases. But I've
   gotten REVERBIT to generate files with a fair amount of DC even *with* this
   simple filter. The Remove DC function in mxv works better. It's the standard
   cmix elliptical filter with 60db atten., stopband at 5 hz, and passband at
   20 hz. But that's overkill for here.
*/

/* local functions */
static void toneset(double, int, double []);
static double tone(double, double []);



REVERBIT::REVERBIT() : Instrument()
{
   in = NULL;
   delarray = rvbarray = NULL;
}

REVERBIT::~REVERBIT()
{
   delete [] in;
   delete [] delarray;
   delete [] rvbarray;
}

int REVERBIT::init(float p[], int n_args)
{
   int   delsamps, rvbsamps, rvin;
   float outskip, inskip, dur, maxdeltime;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   reverbtime = p[4];
   reverbpct = p[5];
   rtchan_delaytime = p[6];
   cutoff = p[7];
   dcblock = n_args > 8 ? (int)p[8] : 1;           /* default is "yes" */

   if (outputchans != 2) {
      die("REVERBIT", "Output must be stereo.");
		return(DONT_SCHEDULE);
	}

   rvin = rtsetinput(inskip, this);
	if (rvin == -1) { // no input
		return(DONT_SCHEDULE);
	}
   if (inputchans > 2) {
      die("REVERBIT", "Can't have more than 2 input channels.");
		return(DONT_SCHEDULE);
	}

   nsamps = rtsetoutput(outskip, dur + reverbtime + RVTSLOP, this);
   insamps = (int)(dur * SR);

   if (reverbtime <= 0.0) {
      die("REVERBIT", "Reverb time must be greater than 0.");
		return(DONT_SCHEDULE);
	}

   if (reverbpct < 0.0 || reverbpct > 1.0) {
      die("REVERBIT", "Reverb percent must be between 0 and 1 inclusive.");
		return(DONT_SCHEDULE);
	}

   if (rtchan_delaytime <= 0.0) {
      die("REVERBIT", "Right chan delay time must be greater than 0.");
		return(DONT_SCHEDULE);
	}

   if (cutoff < 0.0) {
      die("REVERBIT", "Cutoff frequency should be positive (or zero to "
                                                           "disable filter).");
		return(DONT_SCHEDULE);
	}
   else if (cutoff == 0.0)
      advise("REVERBIT", "Low-pass filter disabled.");
   else {
      toneset(cutoff, 1, tonedata);
      advise("REVERBIT", "Low-pass filter cutoff: %g", cutoff);
   }

   maxdeltime = rtchan_delaytime;
   /* If delay time is very short, make delay line longer than necessary. */
   if (rtchan_delaytime < MIN_DELAY)
      maxdeltime *= DELAY_FACTOR;
   delsamps = (int)(maxdeltime * SR + 0.5);
   delarray = new float[delsamps];
   delset(delarray, deltabs, maxdeltime);

   /* Array dimensions taken from lib/rvbset.c (+ 2 extra for caution). */
   rvbsamps = (int)((0.1583 * SR) + 18 + 2);
   rvbarray = new float[rvbsamps];
   rvbset(reverbtime, 0, rvbarray);

   amparray = floc(1);
   if (amparray) {
      int amplen = fsize(1);
      tableset(dur, amplen, amptabs);
   }
   else
      advise("REVERBIT", "Setting phrase curve to all 1's.");

   skip = (int)(SR / (float)resetval);

   prev_in[0] = prev_out[0] = 0.0;         /* for DC-blocker */
   prev_in[1] = prev_out[1] = 0.0;

   return nsamps;
}


int REVERBIT::run()
{
   int   i, branch, rsamps;
   float aamp, insig[2], delsig, rvbsig;
   float out[2];

   if (in == NULL)              /* first time, so allocate it */
      in = new float [RTBUFSAMPS * inputchans];

   Instrument::run();

   rsamps = chunksamps * inputchans;

   rtgetin(in, this, rsamps);

   aamp = amp;                  /* in case amparray == NULL */

   branch = 0;
   for (i = 0; i < rsamps; i += inputchans) {
      if (--branch < 0) {
         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs) * amp;
         branch = skip;
      }
      if (cursamp < insamps) {               /* still taking input from file */
         insig[0] = in[i] * aamp;
         insig[1] = (inputchans == 2) ? in[i + 1] * aamp : insig[0];
      }
      else                                   /* in ring-down phase */
         insig[0] = insig[1] = 0.0;

      rvbsig = -reverbpct * reverb(insig[0] + insig[1], rvbarray);

      if (cutoff)
         rvbsig = tone(rvbsig, tonedata);

      delput(rvbsig, delarray, deltabs);
      delsig = delget(delarray, rtchan_delaytime, deltabs);

      out[0] = insig[0] + rvbsig;
      out[1] = insig[1] + delsig;

      if (dcblock) {
         float tmp_in[2];

         tmp_in[0] = out[0];
         tmp_in[1] = out[1];

         out[0] = tmp_in[0] - prev_in[0] + (0.99 * prev_out[0]);
         prev_in[0] = tmp_in[0];
         prev_out[0] = out[0];

         out[1] = tmp_in[1] - prev_in[1] + (0.99 * prev_out[1]);
         prev_in[1] = tmp_in[1];
         prev_out[1] = out[1];
      }

      rtaddout(out);
      cursamp++;
   }

   return i;
}


/* -------------------------------------------------------------------------- */
/* This filter nabbed from Doug Scott's place program */

/* toneset calculates the coefficients for tone.
      <cutoff> is -3db point in cps
      <flag> will reset history if set to 1
      <data> is an array of 3 doubles, used for bookeeping
   NOTE: REVERBIT doesn't try to use the inverse function cited below.
*/
static void
toneset(double cutoff, int flag, double data[])
{
   double x;

   x = 2.0 - cos(cutoff * PI2 / SR);        /* feedback coeff. */
   data[1] = x - sqrt(x * x - 1.0);      
   data[0] = 1.0 - data[1];                 /* gain coeff. */
   if (cutoff < 0.0)
      data[1] *= -1.0;                      /* inverse function */
   if (flag)
      data[2] = 0.0;
}


/* tone is a simple 1st order recursive lowpass filter
      <data> is an array of 3 doubles, used for bookeeping
*/
static double
tone(double sig, double data[])
{
   data[2] = data[0] * sig + data[1] * data[2];
   return data[2];
}


Instrument *makeREVERBIT()
{
   REVERBIT *inst;

   inst = new REVERBIT();
   inst->set_bus_config("REVERBIT");

   return inst;
}

void rtprofile()
{
   RT_INTRO("REVERBIT", makeREVERBIT);
}

