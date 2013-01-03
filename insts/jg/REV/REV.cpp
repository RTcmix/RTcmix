/* REV - some reverberators from the STK package (by Perry Cook, Gary Scavone,
         and Tim Stilson)

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier (affects signal BEFORE it enters reverb)
   p4 = reverb type (1: PRCRev, 2: JCRev, 3: NRev ... see below)
   p5 = reverb time (seconds)
   p6 = reverb percent (0: dry --> 1: wet)
   p7 = input channel  [optional; default is 0]

   p3 (amplitude) and p6 (reverb percent) can receive dynamic updates from
   a table or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   Reverb types:

     (1) PRCRev (Perry R. Cook)
           2 allpass units in series followed by 2 comb filters in parallel.

     (2) JCRev (John Chowning)
           3 allpass filters in series, followed by 4 comb filters in
           parallel, a lowpass filter, and two decorrelation delay lines
           in parallel at the output.

     (3) NRev (Michael McNabb)
           6 comb filters in parallel, followed by 3 allpass filters, a
           lowpass filter, and another allpass in series, followed by 2
           allpass filters in parallel with corresponding right and left
           outputs.

   John Gibson (jgg9c@virginia.edu), 7/19/99; rev for v4, 7/21/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Instrument.h>
#include "REV.h"
#include <rt.h>
#include <rtdefs.h>


REV :: REV() : Instrument()
{
   in = NULL;
   branch = 0;
}


REV :: ~REV()
{
   delete [] in;
   delete reverb;
}


int REV :: init(double p[], int n_args)
{
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   amp = p[3];
   int rvbtype = (int) p[4];
   float rvbtime = p[5];
   inchan = (int) p[7];

   if (rtsetoutput(outskip, dur + rvbtime, this) == -1)
      return DONT_SCHEDULE;
   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   insamps = (int) (dur * SR + 0.5);

   if (inchan >= inputChannels())
      return die("REV", "You asked for channel %d of a %d-channel file",
                                                    inchan, inputChannels());
   switch (rvbtype) {
      case 1:
         reverb = new PRCRev(SR, rvbtime);
         break;
      case 2:
         reverb = new JCRev(SR, rvbtime);
         break;
      case 3:
         reverb = new NRev(SR, rvbtime);
         break;
      default:
         return die("REV", "Unknown reverb type %d.", rvbtype);
   }

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(SR, dur, lenamp, amptabs);
   }

   return nSamps();
}


int REV :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int REV :: run()
{
   int samps = framesToRun() * inputChannels();

   if (currentFrame() < insamps)
      rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         amp = update(3, insamps);
         if (amparray)
            amp *= tablei(cursamp, amparray, amptabs);
         const double wetdrymix = update(6);
         reverb->setEffectMix(wetdrymix);
         branch = getSkip();
      }

      float insig;
      if (cursamp < insamps)                 // still taking input from file
         insig = in[i + inchan] * amp;
      else                                   // in ring-down phase
         insig = 0.0;

      reverb->tick(insig);

      float out[2];
      if (outputChannels() == 2) {
         out[0] = reverb->lastOutputL();
         out[1] = reverb->lastOutputR();
      }
      else
         out[0] = reverb->lastOutput();

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeREV()
{
   REV *inst;

   inst = new REV();
   inst->set_bus_config("REV");

   return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
   RT_INTRO("REV", makeREV);
}
#endif

