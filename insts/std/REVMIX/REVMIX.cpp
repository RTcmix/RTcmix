/* REVMIX - plays the input file in reverse

   Plays the input file backward for the specified duration, starting
   at the input start time. If you specify a duration that would result
   in an attempt to read before the start of the input file, it will
   shorten the note to prevent this.

   Note that you can't use this instrument with a real-time input
   (microphone or aux bus), only with input from a sound file. (That's
   because the input start time of an inst taking real-time input must
   be zero, but this instrument reads backward from its inskip.)

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = input channel [optional, default is 0]
   p5 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   Ivica "Ico" Bukvic <ico@fuse.net>, 27 May 2000
   (with John Gibson <johngibson@virginia.edu>)
*/
#include <iostream.h>
#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
#include "REVMIX.h"


REVMIX::REVMIX() : Instrument()
{
   in = NULL;
   branch = 0;
}

REVMIX::~REVMIX()
{
   delete [] in;
}

int REVMIX::init(float p[], int n_args)
{
   float outskip, inskip, dur;
	int rvin;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   inchan = n_args > 4 ? (int) p[4] : 0;             /* default is chan 0 */
   pctleft = n_args > 5 ? p[5] : 0.5;                /* default is .5 */

   if (inskip == 0) {
      die("REVMIX", "Input start time must be greater than zero.");
		return(DONT_SCHEDULE);
	}

   if (dur > inskip) {
      warn("REVMIX", "Duration must be greater than input start time. "
                     "Adjusting...");
      dur = inskip;
   }

   nsamps = rtsetoutput(outskip, dur, this);
   rvin = rtsetinput(dur, this);
	if (rvin == -1) { // no input
		return(DONT_SCHEDULE);
	}

   if (inchan >= inputchans) {
      die("REVMIX", "You asked for channel %d of a %d-channel file.",
                                                         inchan, inputchans);
		return(DONT_SCHEDULE);
	}
   amparray = floc(1);
   if (amparray) {
      int amplen = fsize(1);
      tableset(dur, amplen, amptabs);
   }
   else
      advise("REVMIX", "Setting phrase curve to all 1's.");

   skip = (int) (SR / (float) resetval);

   return nsamps;
}


int REVMIX::run()
{
   int   i, samps;
   float aamp;
   float out[2];

   if (in == NULL)                 /* first time, so allocate it */
      in = new float [RTBUFSAMPS * inputchans];

   Instrument::run();

   samps = chunksamps * inputchans;

   rtinrepos(this, -chunksamps, SEEK_CUR);

   rtgetin(in, this, samps);

   aamp = amp;                     /* in case amparray == NULL */

   branch = 0;
   for (i = samps - inputchans; i >=  0; i -= inputchans)  {
      if (--branch < 0) {
         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs) * amp;
         branch = skip;
      }

      out[0] = in[i + inchan] * aamp;

      if (outputchans == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      cursamp++;
   }
   rtinrepos(this, -chunksamps, SEEK_CUR);

   return chunksamps;
}


Instrument *makeREVMIX()
{
   REVMIX *inst;

   inst = new REVMIX();
   inst->set_bus_config("REVMIX");

   return inst;
}

void rtprofile()
{
   RT_INTRO("REVMIX", makeREVMIX);
}

