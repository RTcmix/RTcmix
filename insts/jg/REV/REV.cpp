/* REV - some reverberators from the STK package (by Perry Cook, Gary Scavone,
         and Tim Stilson)

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = reverb type (1: PRCRev, 2: JCRev, 3: NRev ... see below)
   p5 = reverb time (seconds)
   p6 = reverb percent (0: dry --> 1: wet)
   p7 = input channel  [optional; default is 0]

   Assumes function table 1 is an amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses a
   flat amplitude curve. This curve, combined with the amplitude multiplier,
   affect the signal BEFORE it enters the reverb.

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

   John Gibson (jgg9c@virginia.edu), 7/19/99.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "REV.h"
#include <rt.h>
#include <rtdefs.h>



REV :: REV() : Instrument()
{
   in = NULL;
}


REV :: ~REV()
{
   delete [] in;
   delete reverb;
}


int REV :: init(float p[], int n_args)
{
   int   rvbtype;
   float outskip, inskip, dur;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   rvbtype = (int)p[4];
   rvbtime = p[5];
   rvbpct = p[6];
   inchan = (int)p[7];

   nsamps = rtsetoutput(outskip, dur + rvbtime, this);
   rtsetinput(inskip, this);
   insamps = (int)(dur * SR);

   if (inchan >= inputchans)
      die("REV", "You asked for channel %d of a %d-channel file",
                                                          inchan, inputchans);
   switch (rvbtype) {
      case 1:
         reverb = new PRCRev(rvbtime);
         break;
      case 2:
         reverb = new JCRev(rvbtime);
         break;
      case 3:
         reverb = new NRev(rvbtime);
         break;
      default:
         die("REV", "Unknown reverb type %d.", rvbtype);
   }
   reverb->setEffectMix(rvbpct);

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(dur, lenamp, amptabs);
   }
   else
      advise("REV", "Setting phrase curve to all 1's.");

   skip = (int)(SR / (float)resetval);

   return nsamps;
}


int REV :: run()
{
   int   i, branch, rsamps;
   float aamp, insig;
   float out[2];

   if (in == NULL)              /* first time, so allocate it */
      in = new float [RTBUFSAMPS * inputchans];

   Instrument :: run();

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
      if (cursamp < insamps)                 /* still taking input from file */
         insig = in[i + inchan] * aamp;
      else                                   /* in ring-down phase */
         insig = 0.0;

      reverb->tick(insig);

      if (outputchans == 2) {
         out[0] = reverb->lastOutputL();
         out[1] = reverb->lastOutputR();
      }
      else
         out[0] = reverb->lastOutput();

      rtaddout(out);
      cursamp++;
   }

   return i;
}


Instrument *makeREV()
{
   REV *inst;

   inst = new REV();
   inst->set_bus_config("REV");

   return inst;
}


void
rtprofile()
{
   RT_INTRO("REV", makeREV);
}


