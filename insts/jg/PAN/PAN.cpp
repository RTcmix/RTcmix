/* PAN - simple mixing instrument that follows a pan curve

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = input channel [optional, default is 0]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   Function table 2 is the panning curve, described by time,pan pairs.
   <pan> is expressed as the percentage of signal to place in the left
   channel (as in the STEREO instrument), from 0 (0%) to 1 (100%).
   Use gen 24 to make the function table, since it ensures the range
   is [0,1].

   Example:

      makegen(2, 24, 1000, 0,1, 1,0, 3,.5)
      PAN(start=0, inskip=0, dur=3, amp=1, inchan=0)

   This will pan input channel 0 from left to right over the first second.
   Then the sound travels back to the center during the next 2 seconds.

   PAN uses "constant-power" panning to prevent a sense of lost power when
   the pan location moves toward the center.

   John Gibson (jgg9c@virginia.edu), 1/26/00.
*/

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>
#include "PAN.h"
#include <rt.h>
#include <rtdefs.h>

extern "C" {
   #include <ugens.h>
   extern int resetval;
}


PAN :: PAN() : Instrument()
{
}


PAN :: ~PAN()
{
}


int PAN :: init(float p[], short n_args)
{
   float outskip, inskip, dur;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   inchan = n_args > 4 ? (int)p[4] : 0;             /* default is chan 0 */

   nsamps = rtsetoutput(outskip, dur, this);
   rtsetinput(inskip, this);

   if (outputchans != 2) {
      fprintf(stderr, "Output must be stereo!\n");
      exit(1);
   }

   if (inchan >= inputchans) {
      fprintf(stderr, "You asked for channel %d of a %d-channel file.\n",
                                                         inchan, inputchans);
      exit(1);
   }

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(dur, lenamp, amptabs);
   }
   else
      printf("Setting phrase curve to all 1's\n");

   panarray = floc(2);
   if (panarray) {
      int lenpan = fsize(2);
      tableset(dur, lenpan, pantabs);
   }
   else {
      // Note: we won't get here with current floc implementation
      fprintf(stderr, "You haven't made the pan curve function.\n");
      exit(1);
   }

   skip = (int)(SR / (float)resetval);

   return nsamps;
}


int PAN :: run()
{
   int   i, branch, rsamps;
   float aamp, insig;
   float in[MAXBUF], out[2], pan[2];

   Instrument::run();

   rsamps = chunksamps * inputchans;

   rtgetin(in, this, rsamps);

   aamp = amp;                  /* in case amparray == NULL */

   branch = 0;
   for (i = 0; i < rsamps; i += inputchans) {
      if (--branch < 0) {
         float pctleft = tablei(cursamp, panarray, pantabs);
         pan[0] = (float)sqrt((double)pctleft);
         pan[1] = (float)sqrt(1.0 - (double)pctleft);
         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs) * amp;
         branch = skip;
      }
      insig = in[i + inchan] * aamp;

      out[0] = insig * pan[0];
      out[1] = insig * pan[1];

      rtaddout(out);
      cursamp++;
   }

   return i;
}


Instrument *makePAN()
{
   PAN *inst;

   inst = new PAN();
   inst->set_bus_config("PAN");

   return inst;
}


void
rtprofile()
{
   RT_INTRO("PAN", makePAN);
}


