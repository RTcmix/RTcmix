/* BUTTER - time-varying low/high-pass filter

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = type of filter (0: lowpass, 1: highpass)
   p5 = steepness (integer btw 1 and 5, inclusive) [optional, default is 1]
   p6 = balance output and input signals (0:no, 1:yes) [optional, default is 1]
   p7 = input channel [optional, default is 0]
   p8 = percent to left channel [optional, default is .5]

   p5 (steepness) is just the number of filters to add in series.  Using more
   than 1 steepens the slope of the filter.  If you don't set p6 (balance)
   to 1, you'll need to change p3 (amp) to adjust for loss of power caused
   by connecting several filters in series.  Guard your ears!

   p6 (balance) tries to adjust the output of the filter so that it has
   the same power as the input.  This means there's less fiddling around
   with p3 (amp) to get the right amplitude when steepness is > 1.  However,
   it has drawbacks: it can introduce a click at the start of the sound, it
   can cause the sound to pump up and down a bit, and it eats extra CPU time.

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   Function table 2 is the cutoff frequency curve, described by time,cf pairs.
   Use gen 18.

   John Gibson (johgibso@indiana.edu), 12/1/01.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "BUTTER.h"
#include <rt.h>
#include <rtdefs.h>


#define BALANCE_WINDOW_SIZE  10


BUTTER :: BUTTER() : Instrument()
{
   in = NULL;
   balancer = NULL;              // might not use
}


BUTTER :: ~BUTTER()
{
   delete [] in;
   for (int i = 0; i < nfilts; i++)
      delete filt[i];
   if (balancer)
      delete balancer;
}


int BUTTER :: init(float p[], int n_args)
{
   float outskip, inskip, dur;
   const float ringdur = 0.1;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   type = (FiltType)p[4];
   nfilts = n_args > 5 ? (int)p[5] : 1;
   do_balance = n_args > 6 ? (int)p[6] : 1;
   inchan = n_args > 7 ? (int)p[7] : 0;             /* default is chan 0 */
   pctleft = n_args > 8 ? p[8] : 0.5;               /* default is center */

   rtsetinput(inskip, this);
   nsamps = rtsetoutput(outskip, dur + ringdur, this);
   insamps = (int)(dur * SR);

   if (inchan >= inputchans)
      die("BUTTER", "You asked for channel %d of a %d-channel file.",
                                                         inchan, inputchans);
   if (nfilts < 1 || nfilts > MAXFILTS)
      die("BUTTER", "Sharpness (p5) must be an integer between 1 and %d.",
                                                                   MAXFILTS);
   for (int i = 0; i < nfilts; i++)
      filt[i] = new Butter();

   if (do_balance) {
      balancer = new Balance();
      balancer->setWindowSize(BALANCE_WINDOW_SIZE);
      scale = 1.0;
   }
   else
      scale = 1.0 / pow((double)nfilts, 1.8);     // just a guess; needs work

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(dur, lenamp, amptabs);
   }
   else
      advise("BUTTER", "Setting phrase curve to all 1's.");

   cfarray = floc(2);
   if (cfarray) {
      int lencf = fsize(2);
      tableset(dur, lencf, cftabs);
   }
   else     // Note: we won't get here with current floc implementation
      die("BUTTER", "You haven't made the cutoff frequency function.");

   skip = (int)(SR / (float)resetval);

   return nsamps;
}


int BUTTER :: run()
{
   int   i, j, branch, rsamps;
   float aamp, cf, insig;
   float out[2];

   if (in == NULL)            /* first time, so allocate it */
      in = new float [RTBUFSAMPS * inputchans];

   Instrument :: run();

   rsamps = chunksamps * inputchans;

   if (cursamp < insamps)
      rtgetin(in, this, rsamps);

   aamp = amp * scale;        /* in case amparray == NULL */

   branch = 0;
   for (i = 0; i < rsamps; i += inputchans) {
      if (--branch < 0) {
         cf = tablei(cursamp, cfarray, cftabs);
         if (cf <= 0.0)
            cf = 1.0;
         if (cf > SR * 0.5)
            cf = SR * 0.5;
         if (type == LowPass) {
            for (j = 0; j < nfilts; j++)
               filt[j]->setLowPass(cf);
         }
         else if (type == HighPass) {
            for (j = 0; j < nfilts; j++)
               filt[j]->setHighPass(cf);
         }
         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs) * amp;

         branch = skip;
      }
      if (cursamp < insamps)
         insig = in[i + inchan] * aamp;
      else
         insig = 0.0;

      out[0] = insig;
      for (j = 0; j < nfilts; j++)
         out[0] = filt[j]->tick(out[0]);

      if (do_balance)
         out[0] = balancer->tick(out[0], insig);

      if (outputchans == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      cursamp++;
   }

   return i;
}


Instrument *makeBUTTER()
{
   BUTTER *inst;

   inst = new BUTTER();
   inst->set_bus_config("BUTTER");

   return inst;
}


void
rtprofile()
{
   RT_INTRO("BUTTER", makeBUTTER);
}


