/* SHAPE - waveshaping instrument

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = input channel [optional, default is 0]
   p5 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   JGG <johgibso@indiana.edu>, 3 Jan 2002
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>
#include "SHAPE.h"
#include <rt.h>
#include <rtdefs.h>


SHAPE :: SHAPE() : Instrument()
{
   in = NULL;
   branch = 0;
}


SHAPE :: ~SHAPE()
{
   delete [] in;
   delete shaper;
}


int SHAPE :: init(float p[], int n_args)
{
   float outskip, inskip, dur;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   inchan = n_args > 4 ? (int) p[4] : 0;             /* default is chan 0 */
   pctleft = n_args > 5 ? p[5] : 0.5;                /* default is .5 */

   nsamps = rtsetoutput(outskip, dur, this);
   rtsetinput(inskip, this);

   if (inchan >= inputchans)
      die("SHAPE", "You asked for channel %d of a %d-channel file.",
                                                         inchan, inputchans);

   amparray = floc(1);
   if (amparray) {
      int len = fsize(1);
      tableset(dur, len, amptabs);
   }
   else
      advise("SHAPE", "Setting phrase curve to all 1's.");

   shaper = new WavShape(1.0);
   transfer_func = floc(2);
   if (transfer_func) {
      int len = fsize(2);
      shaper->setTransferFunc(transfer_func, len);
   }
   else
      die("SHAPE", "You need a transfer function in gen slot 2.");

   skip = (int) (SR / (float) resetval);
   aamp = amp;                  /* in case amparray == NULL */

   return nsamps;
}


int SHAPE :: run()
{
   int   i, samps;
   float insig;
   float out[2];

   if (in == NULL)
      in = new float [RTBUFSAMPS * inputchans];

   Instrument::run();

   samps = chunksamps * inputchans;
   rtgetin(in, this, samps);

   for (i = 0; i < samps; i += inputchans) {
      if (--branch < 0) {
         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs) * amp;
         branch = skip;
      }

      insig = in[i + inchan];
      out[0] = shaper->tick(insig * (1.0 / 32768.0));
      out[0] *= 32768.0 * aamp;

      if (outputchans == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      cursamp++;
   }

   return chunksamps;
}


Instrument *makeSHAPE()
{
   SHAPE *inst;

   inst = new SHAPE();
   inst->set_bus_config("SHAPE");

   return inst;
}


void rtprofile()
{
   RT_INTRO("SHAPE", makeSHAPE);
}


