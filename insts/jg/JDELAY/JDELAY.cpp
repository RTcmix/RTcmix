/* JDELAY: Regenerating delay instrument, adapted from DELAY.
 *    The differences between JDELAY and DELAY are:
 *       - JDELAY uses interpolating delay line fetch and
 *         longer-than-necessary delay line, both of
 *         which make it sound less buzzy for audio-range delays
 *       - provides a simple low-pass filter
 *       - provides control over wet/dry mix
 *       - provides a DC blocking filter
 *       - by default the delay is "post-fader," as before, but there's
 *         a pfield switch to make it "pre-fader."
 *
 * Parameters:
 *    p0 = output start time
 *    p1 = input start time
 *    p2 = input duration
 *    p3 = amplitude multiplier
 *    p4 = delay time
 *    p5 = regeneration multiplier (must be >= -1.0 and <= 1.0)
 *    p6 = ring-down duration
 *    p7 = cutoff freq for low-pass filter (in cps)  (0 to disable filter)
 *    p8 = wet/dry mix (0: dry -> 1: wet)
 *    p9 = input channel number [optional, default is 0]
 *    p10 = stereo spread (0-1, % to left chan) [optional, default is .5]
 *    p11 = pre-fader send (0: No, 1: Yes) [optional, default is No]
 *    p12 = apply DC blocking filter (0: No, 1: Yes) [optional, default is Yes]
 *          (DC bias can affect sounds made with high regeneration setting.)
 *
 *    Assumes function slot 1 is the amplitude envelope (see above)
 *    Or you can just call setline. If no setline or function table 1, uses
 *    flat amplitude curve.
 *
 *    If pre-fader send is set to 1, sends input signal to delay line with
 *    no attenuation. Then p3 (amp multiplier) and setline controls entire
 *    note, including delay ring-down.
 *
 *    John Gibson (jgg9c@virginia.edu), 6/23/99
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "JDELAY.h"
#include <rt.h>
#include <rtdefs.h>


#define DELAY_FACTOR   2       /* Determines length of delay line */
#define MIN_DELAY      .10     /* Uses extra-large delay line below this */

/* The DC-blocking code is from Perry Cook's STK (STK98v2/DCBlock.cpp).
   It works pretty well (and quickly) for all but extreme cases. But I've
   gotten JDELAY to generate files with a fair amount of DC even *with* this
   simple filter. The Remove DC function in mxv works better. It's the standard
   cmix elliptical filter with 60db atten., stopband at 5 hz, and passband at
   20 hz. But that's overkill for here.
*/

/* local functions */
static void toneset(double, int, double []);
static double tone(double, double []);



JDELAY::JDELAY() : Instrument()
{
   in = NULL;
   delarray = NULL;
}


JDELAY::~JDELAY()
{
   delete [] in;
   delete [] delarray;
}


int JDELAY::init(float p[], int n_args)
{
   int   delsamps;
   float outskip, inskip, dur, ringdur;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   wait = p[4];
   regen = p[5];
   ringdur = p[6];
   cutoff = p[7];
   percent_wet = p[8];
   inchan = n_args > 9 ? (int)p[9] : 0;
   spread = n_args > 10 ? p[10] : 0.5;             /* default is center */
   prefadersend = n_args > 11 ? (int)p[11] : 0;    /* default is "no" */
   dcblock = n_args > 12 ? (int)p[12] : 1;         /* default is "yes" */

   rtsetinput(inskip, this);
   nsamps = rtsetoutput(outskip, dur + ringdur, this);
   insamps = (int)(dur * SR);

   if (inchan >= inputchans)
      die("JDELAY", "You asked for channel %d of a %d-channel file.",
                                                         inchan, inputchans);

   if (regen < -1.0 || regen > 1.0)
      die("JDELAY", "Regeneration multiplier must be between -1 and 1.");

   if (cutoff < 0.0)
      die("JDELAY",
              "Cutoff freq. should be positive (or zero to disable filter).");
   else if (cutoff == 0.0)
      advise("JDELAY", "Low-pass filter disabled.");
   else {
      toneset(cutoff, 1, tonedata);
      advise("JDELAY", "Low-pass filter cutoff: %g", cutoff);
   }

   if (percent_wet < 0.0 || percent_wet > 1.0)
      die("JDELAY", "Wet/dry mix must be between 0 and 1 inclusive.");

   delsamps = (int)(wait * SR + 0.5);
   /* If delay time is very short, make delay line longer than necessary;
      makes output sound less buzzy.
   */
   if (wait < MIN_DELAY)
      delsamps = (int)((wait * SR * DELAY_FACTOR) + 0.5);

   delarray = new float[delsamps];

   if (wait < MIN_DELAY)
      delset(delarray, deltabs, wait * DELAY_FACTOR);
   else
      delset(delarray, deltabs, wait);

   amptable = floc(1);
   if (amptable) {
      int amplen = fsize(1);
      if (prefadersend)
         tableset(dur + ringdur, amplen, amptabs);
      else
         tableset(dur, amplen, amptabs);
   }
   else
      advise("JDELAY", "Setting phrase curve to all 1's.");

   skip = (int)(SR / (float)resetval);

   prev_in = prev_out = 0.0;         /* for DC-blocker */

   return nsamps;
}


int JDELAY::run()
{
   int   i, branch, rsamps;
   float aamp, insig, delsig;
   float out[2];

   if (in == NULL)              /* first time, so allocate it */
      in = new float [RTBUFSAMPS * inputchans];

   Instrument::run();

   rsamps = chunksamps * inputchans;

   rtgetin(in, this, rsamps);

   aamp = amp;                  /* in case amptable == NULL */

   branch = 0;
   for (i = 0; i < rsamps; i += inputchans) {

      delsig = dliget(delarray, wait, deltabs) * regen;

      if (cursamp < insamps) {               /* still taking input from file */
         if (--branch < 0) {
            if (amptable)
               aamp = tablei(cursamp, amptable, amptabs) * amp;
            branch = skip;
         }

         insig = in[i + inchan];
         if (prefadersend) {
            delput(insig + delsig, delarray, deltabs);
            out[0] = insig * (1.0 - percent_wet) + delsig * percent_wet;
            out[0] *= aamp;
         }
         else {
            insig *= aamp;
            delput(insig + delsig, delarray, deltabs);
            out[0] = insig * (1.0 - percent_wet) + delsig * percent_wet;
         }
      }
      else {                                 /* in ring-down phase */
         delput(delsig, delarray, deltabs);
         out[0] = delsig * percent_wet;
         if (prefadersend) {
            if (--branch < 0) {
               if (amptable)
                  aamp = tablei(cursamp, amptable, amptabs) * amp;
               branch = skip;
            }
            out[0] *= aamp;
         }
      }

      /* NOTE: We filter the sig *after* putting it in the delay line.
               Otherwise, pitch drops with low cutoff freqs.
      */
      if (cutoff)
         out[0] = tone(out[0], tonedata);

      if (dcblock) {
         float tmp_in = out[0];

         out[0] = tmp_in - prev_in + (0.99 * prev_out);
         prev_in = tmp_in;
         prev_out = out[0];
      }

      if (outputchans == 2) {
         out[1] = out[0] * (1.0 - spread);
         out[0] *= spread;
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
   NOTE: JDELAY doesn't try to use the inverse function cited below.
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


Instrument *makeJDELAY()
{
   JDELAY *inst;

   inst = new JDELAY();
   inst->set_bus_config("JDELAY");

   return inst;
}

void rtprofile()
{
   RT_INTRO("JDELAY", makeJDELAY);
}

