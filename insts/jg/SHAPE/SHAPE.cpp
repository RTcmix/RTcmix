/* SHAPE - waveshaping instrument

   This differs from insts.std/WAVESHAPE in that it accepts input from
   a file or bus, and it offers a different way of performing amplitude
   normalization.

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier *
   p4 = minimum distortion index
   p5 = maximum distortion index
   p6 = reference to an amplitude normalization table, or 0 for no norm. **
   p7 = input channel [optional, default is 0.5]
   p8 = percent of signal to left output channel [optional, default is .5]
   p9 = reference to waveshaping transfer function table [optional; if missing,
        must use gen 2] ***
   p10 = index guide [optional; if missing, can use gen 3] ****

   p3 (amplitude), p4 (min index), p5 (max index), p8 (pan) and p10 (index)
   can receive dynamic updates from a table or real-time control source.

   The amplitude normalization function allows you to decouple amplitude and
   timbral change, to some extent.  (Read about this in the Roads "Computer
   Music Tutorial" chapter on waveshaping.)  The table maps incoming signal
   amplitudes, on the X axis, to amplitude multipliers, on the Y axis.  The
   multipliers should be from 0 to 1.  The amplitude multipliers are applied
   to the output signal, along with whatever overall amplitude curve is in
   effect.  Generally, you'll want a normalization curve that starts high and
   moves lower (see SHAPE2.sco) for the higher signal amplitudes.  This will
   keep the bright and dark timbres more equal in amplitude.

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   ** If p6 is a non-zero constant, then that is the number of an old-style
   gen table slot to use for the amplitude normalization function.

   *** If p9 is missing, you must use an old-style gen table 2 for the
   waveshaping transfer function.

   **** If p10 is missing, you can use an old-style gen table 3 for the
   distortion index curve.  Otherwise, a constant 1.0 will be used.


   John Gibson <johgibso at indiana dot edu>, 3 Jan 2002; rev for v4, 7/21/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <Instrument.h>
#include <PField.h>
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
   nargs = n_args;
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   amp = p[3];
   min_index = p[4];
   max_index = p[5];
   int ampnorm_genno = (int) p[6];
   inchan = n_args > 7 ? (int) p[7] : 0;             /* default is chan 0 */

   if (n_args < 7)
      return die("SHAPE", "Needs at least 7 arguments.");

   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;
   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;

   if (inchan >= inputChannels())
      return die("SHAPE", "You asked for channel %d of a %d-channel file.",
                                                   inchan, inputChannels());

   if (max_index < min_index)
      return die("SHAPE",
                 "Max. distortion index must not be less than min. index.");

   double *function = floc(1);
   if (function) {
      int len = fsize(1);
      amp_table = new TableL(SR, dur, function, len);
   }

   function = NULL;
   int tablelen = 0;
   if (n_args > 9) {    // handle table coming in as optional p9 TablePField
      function = (double *) getPFieldTable(9, &tablelen);
   }
   if (function == NULL) {
      function = floc(2);
      if (function == NULL)
         return die("SHAPE", "Either use the transfer function pfield (p9) "
                    "or make an old-style gen function in slot 2.");
      tablelen = fsize(2);
   }
   shaper = new WavShape();
   shaper->setTransferFunc(function, tablelen);

   function = NULL;
	if (n_args < 11) {		// no p10 guide PField, must use gen table
      function = floc(3);
      if (function) {
         int len = fsize(3);
         index_table = new TableL(SR, dur, function, len);
      }
      else
         rtcmix_advise("SHAPE", "Setting distortion index curve to all 1's.");
   }

   /* Construct the <ampnorm> WavShape object if (1) p6 is a TablePField, or
      (2) if p6 is non-zero, in which case use p6 as the gen slot for the
      amp norm function.  If p6 is zero, then don't construct <ampnorm>.
   */
   function = NULL;
   const PField &field = getPField(6);
   tablelen = field.values();
   function = (double *) field;
   if (function == NULL) {    // no table pfield
      if (ampnorm_genno > 0) {
         function = floc(ampnorm_genno);
         if (function == NULL)
            return die("SHAPE", "You specified table %d as the amplitude "
                    "normalization function, but you didn't create the table.",
                    ampnorm_genno);
         tablelen = fsize(ampnorm_genno);
      }
   }
   if (function) {
      ampnorm = new WavShape();
      ampnorm->setTransferFunc(function, tablelen);
   }

   dcblocker = new DCBlock();

   skip = (int) (SR / (float) resetval);

   return nSamps();
}


int SHAPE :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


void SHAPE :: doupdate()
{
   double p[11];
   update(p, 11, kAmp | kMinIndex | kMaxIndex | kPan | kIndex);

   amp = p[3];
   if (amp_table)
      amp *= amp_table->tick(currentFrame(), 1.0);

   min_index = p[4];
   max_index = p[5];
   if (max_index < min_index)
      max_index = min_index;

   float rawindex;
   if (nargs > 10)         // use index PField
      rawindex = p[10];
   else if (index_table)   // use gen 3
      rawindex = index_table->tick(currentFrame(), 1.0);
   else
      rawindex = 1.0;
   index = min_index + (rawindex * (max_index - min_index));

   // scale index vals to range [-1, 1] in order to use WavShape
   if (ampnorm)
      norm_index = (index / (max_index / 2.0)) - 1.0;

   pctleft = nargs > 8 ? p[8] : 0.5;                // default is .5
}


int SHAPE :: run()
{
   const int samps = framesToRun() * inputChannels();
   rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         doupdate();
         branch = skip;
      }

      // NB: WavShape deals with samples in range [-1, 1].
      float insig = in[i + inchan] * (1.0 / 32768.0);
      float outsig = shaper->tick(insig * index);
      if (outsig) {
         if (ampnorm)
            outsig = dcblocker->tick(outsig) * ampnorm->tick(norm_index);
         else
            outsig = dcblocker->tick(outsig);
      }
      float out[2];
      out[0] = outsig * amp * 32768.0;

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

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("SHAPE", makeSHAPE);
}
#endif

