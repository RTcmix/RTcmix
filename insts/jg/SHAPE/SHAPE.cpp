/* SHAPE - waveshaping instrument

   This differs from insts.std/WAVESHAPE in that it accepts input from
   a file or bus, and it offers a different way of performing amplitude
   normalization.

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = minimum distortion index
   p5 = maximum distortion index
   p6 = table number of amplitude normalization function, or 0 for no norm.
   p7 = input channel [optional, default is 0]
   p8 = percent of signal to left output channel [optional, default is .5]

   Function tables:
      1  amplitude curve (or use setline)
         If no setline or function table 1, uses flat amplitude curve.
      2  waveshaping transfer function
      3  distortion index curve

   NOTES:
      - The amplitude normalization function allows you to decouple
        amplitude and timbral change, to some extent.  (Read about this
        in the Roads "Computer Music Tutorial" chapter on waveshaping.)
        The table maps incoming signal amplitudes, on the X axis, to
        amplitude multipliers, on the Y axis.  The multipliers should be
        from 0 to 1.  The amplitude multipliers are applied to the 
        output signal, along with whatever overall amplitude curve is
        in effect.  Generally, you'll want a curve that starts high and
        moves lower (see SHAPE2.sco) for the higher signal amplitudes.
        This will keep the bright and dark timbres more equal in amplitude.
        But anything's possible.

   John Gibson <johgibso at indiana dot edu>, 3 Jan 2002
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
   amp_table = NULL;
   ampnorm = NULL;
   index_table = NULL;
   norm_index = 0.0;
   branch = 0;
}


SHAPE :: ~SHAPE()
{
   delete [] in;
   delete amp_table;
   delete index_table;
   delete shaper;
   delete ampnorm;
   delete dcblocker;
}


int SHAPE :: init(double p[], int n_args)
{
   int   ampnorm_genno;
   float outskip, inskip, dur;
   float *function;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   min_index = p[4];
   max_index = p[5];
   ampnorm_genno = (int) p[6];
   inchan = n_args > 7 ? (int) p[7] : 0;             /* default is chan 0 */
   pctleft = n_args > 8 ? p[8] : 0.5;                /* default is .5 */

   nsamps = rtsetoutput(outskip, dur, this);
   if (rtsetinput(inskip, this) != 0)
      return DONT_SCHEDULE;

   if (inchan >= inputChannels())
      return die("SHAPE", "You asked for channel %d of a %d-channel file.",
                                                   inchan, inputChannels());

   if (max_index < min_index)
      return die("SHAPE",
                 "Max. distortion index must not be less than min. index.");

   function = floc(1);
   if (function) {
      int len = fsize(1);
      amp_table = new TableL(dur, function, len);
   }
   else
      advise("SHAPE", "Setting amplitude curve to all 1's.");

   shaper = new WavShape();
   function = floc(2);
   if (function) {
      int len = fsize(2);
      shaper->setTransferFunc(function, len);
   }
   else
      return die("SHAPE", "You haven't made the transfer function (table 2).");

   function = floc(3);
   if (function) {
      int len = fsize(3);
      index_table = new TableL(dur, function, len);
   }
   else {
      advise("SHAPE", "Setting distortion index curve to all 1's.");
      index = 1.0;
   }

   if (ampnorm_genno > 0) {
      function = floc(ampnorm_genno);
      if (function) {
         ampnorm = new WavShape();
         int len = fsize(ampnorm_genno);
         ampnorm->setTransferFunc(function, len);
      }
      else
         return die("SHAPE", "You specified table %d as the amplitude "
                    "normalization function, but you didn't create the table.",
                    ampnorm_genno);
   }

   dcblocker = new DCBlock();

   skip = (int) (SR / (float) resetval);
   aamp = amp;                  /* in case amparray == NULL */

   return nsamps;
}


int SHAPE :: run()
{
   int   i, samps;
   float insig, outsig;
   float out[2];

   if (in == NULL)
      in = new float [RTBUFSAMPS * inputChannels()];

   samps = framesToRun() * inputChannels();
   rtgetin(in, this, samps);

   for (i = 0; i < samps; i += inputChannels()) {
      if (--branch < 0) {
         if (amp_table)
            aamp = amp_table->tick(currentFrame(), amp);
         if (index_table)
            index = index_table->tick(currentFrame(), 1.0);
         index = min_index + (index * (max_index - min_index));
         if (ampnorm)
            /* scale index vals to range [-1, 1] in order to use WavShape */
            norm_index = (index / (max_index / 2.0)) - 1.0;
         branch = skip;
      }

      /* NB: WavShape deals with samples in range [-1, 1]. */
      insig = in[i + inchan] * (1.0 / 32768.0);
      outsig = shaper->tick(insig * index);
      if (outsig) {
         if (ampnorm)
            outsig = dcblocker->tick(outsig) * ampnorm->tick(norm_index);
         else
            outsig = dcblocker->tick(outsig);
      }
      out[0] = outsig * aamp * 32768.0;

      if (outputChannels() == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      increment();
   }

   return framesToRun();
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


