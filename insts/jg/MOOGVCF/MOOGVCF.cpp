/* MOOGVCF - a 24dB/octave resonant lowpass filter

   This is based on the design by Stilson and Smith (CCRMA), as modified
   by Paul Kellett <paul.kellett@maxim.abel.co.uk> (described in the source
   code archives of the Music DSP site -- musicdsp.org).

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = input channel [optional, default is 0]
   p5 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   Function table 2 is the cutoff frequency curve.

   Function table 3 is the resonance curve.  Should range between 0 and 1.
   Easiest to use gen 18, and make sure values are in that range.

   JGG <johgibso@indiana.edu>, 22 May 2002
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>
#include "MOOGVCF.h"
#include <rt.h>
#include <rtdefs.h>


MOOGVCF :: MOOGVCF() : Instrument()
{
   in = NULL;
   branch = 0;
   b0 = b1 = b2 = b3 = b4 = 0.0;
}


MOOGVCF :: ~MOOGVCF()
{
   delete [] in;
}


int MOOGVCF :: init(float p[], int n_args)
{
   float outskip, inskip, dur, ringdown;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   inchan = n_args > 4 ? (int) p[4] : 0;             /* default is chan 0 */
   pctleft = n_args > 5 ? p[5] : 0.5;                /* default is .5 */

   ringdown = 0.2;      /* just a guess, for now */
   nsamps = rtsetoutput(outskip, dur + ringdown, this);
   rtsetinput(inskip, this);

   if (inchan >= inputchans)
      die("MOOGVCF", "You asked for channel %d of a %d-channel file.",
                                                         inchan, inputchans);

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(dur, lenamp, amptabs);
   }
   else
      advise("MOOGVCF", "Setting phrase curve to all 1's.");

   cfarray = floc(2);
   if (cfarray) {
      int lencf = fsize(2);
      tableset(dur, lencf, cftabs);
   }
   else {   // NB: We'll never get here with current floc! 
      cf = 1000.0;
      warn("MOOGVCF", "Without table 2, cutoff frequency set to 1kHz.");
   }

   resarray = floc(3);
   if (resarray) {
      int lenres = fsize(3);
      tableset(dur, lenres, restabs);
   }
   else {   // NB: We'll never get here with current floc! 
      res = 0.5;
      warn("MOOGVCF", "Without table 3, resonance set to 0.5.");
   }

   make_coefficients();

   skip = (int) (SR / (float) resetval);

   aamp = amp;                  /* in case amparray == NULL */

   return nsamps;
}


int MOOGVCF :: run()
{
   int   i, samps;
   float insig, t1, t2;
   float out[2];

   if (in == NULL)
      in = new float [RTBUFSAMPS * inputchans];

   Instrument::run();

   samps = chunksamps * inputchans;
   rtgetin(in, this, samps);

   for (i = 0; i < samps; i += inputchans) {       /* loop for each frame */
      if (--branch < 0) {
         float newcf = cf, newres = res;
         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs) * amp;
         if (cfarray)
            newcf = tablei(cursamp, cfarray, cftabs);
         if (resarray)
            newres = tablei(cursamp, resarray, restabs);
         if (newcf != cf || newres != res) {
            cf = newcf;
            res = newres;
            make_coefficients();
         }
         branch = skip;
      }

      insig = in[i + inchan] * aamp;

      /* Bring (normally) into range [-1, 1], because filter code
         assumes this.
      */
      insig /= 32768.0;

      insig -= q * b4;                             /* feedback */

      /* four cascaded onepole filters (bilinear transform) */
      t1 = b1;  b1 = (insig + b0) * p - b1 * f;
      t2 = b2;  b2 = (b1 + t1)    * p - b2 * f;
      t1 = b3;  b3 = (b2 + t2)    * p - b3 * f;
                b4 = (b3 + t1)    * p - b4 * f;
      b4 = b4 - b4 * b4 * b4 * 0.166667;           /* clipping */
      b0 = insig;

#if defined(HIGHPASS)      /* but only 6dB/oct */
      out[0] = insig - b4;
#elif defined(BANDPASS)    /* 6dB/oct */
      out[0] = 3.0 * (b3 - b4);
#else
      out[0] = b4;
#endif
      out[0] *= 32768.0;

      if (outputchans == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      cursamp++;
   }

   return chunksamps;
}


Instrument *makeMOOGVCF()
{
   MOOGVCF *inst;

   inst = new MOOGVCF();
   inst->set_bus_config("MOOGVCF");

   return inst;
}


void rtprofile()
{
   RT_INTRO("MOOGVCF", makeMOOGVCF);
}


