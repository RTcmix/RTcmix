/* FREEVERB - a reverberator

   This reverb instrument is based on Freeverb, by Jezar at Dreampoint
   (http://www.dreampoint.co.uk/~jzracc/freeverb.htm).

   p0  = output start time
   p1  = input start time
   p2  = input duration
   p3  = amplitude multiplier
   p4  = room size (0-1.07143 ... don't ask)
   p5  = damp (0-100%)
   p6  = dry (0-?)
   p7  = wet (0-?)
   p8  = width (0-100%)
   p9  = ringdown duration (seconds)

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.  The curve is applied to the input sound *before*
   it enters the reverberator.

   John Gibson <johngibson@virginia.edu>, 2 Feb 2001
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>
#include "FREEVERB.h"
#include <rt.h>
#include <rtdefs.h>


FREEVERB :: FREEVERB() : Instrument()
{
   in = NULL;
   branch = 0;
}


FREEVERB :: ~FREEVERB()
{
   delete [] in;
}


int FREEVERB :: init(float p[], int n_args)
{
   float outskip, inskip, dur, roomsize, damp, dry, wet, width, max_roomsize;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   roomsize = p[4];
   damp = p[5];
   dry = p[6];
   wet = p[7];
   width = p[8];
   ringdur = p[9];

   /* Keep reverb comb feedback <= 1.0 */
   max_roomsize = (1.0 - offsetroom) / scaleroom;
   if (roomsize < 0.0)
      die("FREEVERB", "Room size must be between 0 and %g.", max_roomsize);
   if (roomsize > max_roomsize) {
      roomsize = max_roomsize;
      advise("FREEVERB", "Room size cannot be greater than %g. Adjusting...",
             max_roomsize);
   }

   if (damp < 0.0 || damp > 100.0)
      die("FREEVERB", "Damp must be between 0 and 100%%.");
   if (width < 0.0 || width > 100.0)
      die("FREEVERB", "Width must be between 0 and 100%%.");

   rtsetinput(inskip, this);
   nsamps = rtsetoutput(outskip, dur + ringdur, this);
   insamps = (int)(dur * SR);

/* legal chan configs:  1in1out, 2in2out, 1in2out */
   if (inputchans == 2 && outputchans == 1)
      die("FREEVERB", "If input is stereo, output must be stereo.");

   rvb = new revmodel();

   rvb->setroomsize(roomsize);
   rvb->setdamp(damp * 0.01);
   rvb->setdry(dry);
   rvb->setwet(wet);
   rvb->setwidth(width * 0.01);

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(dur, lenamp, amptabs);
   }
   else
      advise("FREEVERB", "Setting phrase curve to all 1's.");

   aamp = amp;                  /* in case amparray == NULL */

   skip = (int) (SR / (float) resetval);

   return nsamps;
}


int FREEVERB :: run()
{
   int   samps;
   float *inL, *inR, *outL, *outR;

   if (in == NULL)
      in = new float [RTBUFSAMPS * inputchans];

   Instrument :: run();

   inL = in;
   inR = inputchans > 1 ? in + 1 : in;
   outL = outbuf;
   outR = outputchans > 1 ? outbuf + 1 : outbuf;

   samps = chunksamps * inputchans;

   if (cursamp < insamps)
      rtgetin(in, this, samps);

   /* Scale input signal by amplitude multiplier and setline curve. */
   for (int i = 0; i < samps; i += inputchans) {
      if (cursamp < insamps) {           /* still taking input from file */
         if (--branch < 0) {
            if (amparray)
               aamp = tablei(cursamp, amparray, amptabs) * amp;
            branch = skip;
         }
         in[i] *= aamp;
         if (inputchans == 2)
            in[i + 1] *= aamp;
      }
      else {                             /* in ringdown phase */
         in[i] = 0.0;
         if (inputchans == 2)
            in[i + 1] = 0.0;
      }
      cursamp++;
   }

   /* Hand off to Freeverb to do the actual work. */
   rvb->processreplace(inL, inR, outL, outR, chunksamps, inputchans,
                                                         outputchans);

   return chunksamps;
}


Instrument *makeFREEVERB()
{
   FREEVERB *inst;

   inst = new FREEVERB();
   inst->set_bus_config("FREEVERB");

   return inst;
}


void rtprofile()
{
   RT_INTRO("FREEVERB", makeFREEVERB);
}


