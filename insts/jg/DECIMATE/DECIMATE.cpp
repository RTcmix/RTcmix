/* DECIMATE - reduce number of bits used to represent sound

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier (before decimation)
   p4 = number of bits to use (1 to 16)
   p5 = low-pass filter cutoff frequency (or 0 to bypass)
         [optional, default is 0]
   p6 = input channel [optional, default is 0]
   p7 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.  This is applied AFTER the decimation and filter.

   JGG <johgibso@indiana.edu>, 3 Jan 2002
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>
#include "DECIMATE.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG
//#define DEBUG2


DECIMATE :: DECIMATE() : Instrument()
{
   in = NULL;
   lpfilt = NULL;    /* might not use */
   branch = 0;
   aamp = 1.0;       /* in case amparray == NULL */
}


DECIMATE :: ~DECIMATE()
{
   delete [] in;
   if (lpfilt)
      delete lpfilt;
}


int DECIMATE :: init(float p[], int n_args)
{
   int   bits;
   float outskip, inskip, dur, cutoff;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   bits = (int) p[4];
   cutoff = n_args > 5 ? p[5] : 0.0;                 /* default is bypass */
   inchan = n_args > 6 ? (int) p[6] : 0;             /* default is chan 0 */
   pctleft = n_args > 7 ? p[7] : 0.5;                /* default is .5 */

   nsamps = rtsetoutput(outskip, dur, this);
   rtsetinput(inskip, this);

   if (inchan >= inputchans)
      die("DECIMATE", "You asked for channel %d of a %d-channel file.",
                                                         inchan, inputchans);
   if (bits > 16 || bits < 1)
      die("DECIMATE", "Bits must be between 1 and 16.");

   /* NB: This depends on int being 32 bits! */
   mask = (int) ( ((int) pow(2.0, (double) bits) - 1) << (16 - bits) );
   mask |= 0xFFFF0000;
   if (bits == 16)
      bias = 0;
   else {
      int power = (16 - bits) - 1;
      bias = (int) pow(2.0, (double) power);
   }
#ifdef DEBUG
   printf("mask = 0x%X, bias = %d\n", mask, bias);
#endif

   amparray = floc(1);
   if (amparray) {
      int len = fsize(1);
      tableset(dur, len, amptabs);
   }
   else
      advise("DECIMATE", "Setting phrase curve to all 1's.");

   if (cutoff < 0.0 || cutoff > SR * 0.5)
      die("DECIMATE", "Cutoff frequency must be between 0 and %g.", SR * 0.5);

   if (cutoff > 0.0) {
      lpfilt = new Butter();
      lpfilt->setLowPass(cutoff);
   }

   skip = (int) (SR / (float) resetval);

   return nsamps;
}


int DECIMATE :: run()
{
   int   i, samps, isig;
   float sig, out[2];

   if (in == NULL)
      in = new float [RTBUFSAMPS * inputchans];

   Instrument::run();

   samps = chunksamps * inputchans;
   rtgetin(in, this, samps);

   for (i = 0; i < samps; i += inputchans) {
      if (--branch < 0) {
         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs);
         branch = skip;
      }

      sig = in[i + inchan] * amp;
      isig = (int) sig;
      out[0] = (float) ((isig & mask) + bias);
#ifdef DEBUG2
      printf("%f -> %d -> %f\n", sig, isig, out[0]);
#endif
      if (lpfilt)
         out[0] = lpfilt->tick(out[0]);

      out[0] *= aamp;

      if (outputchans == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      cursamp++;
   }

   return chunksamps;
}


Instrument *makeDECIMATE()
{
   DECIMATE *inst;

   inst = new DECIMATE();
   inst->set_bus_config("DECIMATE");

   return inst;
}


void rtprofile()
{
   RT_INTRO("DECIMATE", makeDECIMATE);
}


