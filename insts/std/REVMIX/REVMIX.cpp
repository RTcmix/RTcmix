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

   p3 (amplitude) and p5 (pan) can receive dynamic updates from a table or
   real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   Ivica "Ico" Bukvic <ico@fuse.net>, 27 May 2000
   (with John Gibson <johngibson@virginia.edu>)
   rev. for v4.0, JGG, 7/9/04
*/
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

int REVMIX::init(double p[], int n_args)
{
   nargs = n_args;

   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   inchan = n_args > 4 ? (int) p[4] : 0;        // default is chan 0

   if (inskip == 0)
      return die("REVMIX", "Input start time must be greater than zero.");

   if (dur > inskip) {
      rtcmix_warn("REVMIX", "Duration must be greater than input start time. "
                     "Adjusting...");
      dur = inskip;
   }

   if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
   if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;  // no input

   if (inchan >= inputChannels())
      return die("REVMIX", "You asked for channel %d of a %d-channel file.",
                                                      inchan, inputChannels());

   amparray = floc(1);
   if (amparray) {
      int amplen = fsize(1);
      tableset(SR, dur, amplen, amptabs);
   }

   skip = (int) (SR / (float) resetval);

   return nSamps();
}


int REVMIX::configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int REVMIX::run()
{
   int samps = framesToRun() * inputChannels();

   rtinrepos(this, -framesToRun(), SEEK_CUR);

   rtgetin(in, this, samps);

   for (int i = samps - inputChannels(); i >=  0; i -= inputChannels())  {
      if (--branch <= 0) {
         double p[nargs];
         update(p, nargs);
         amp = p[3];
         if (amparray)
            amp *= tablei(currentFrame(), amparray, amptabs);
         pctleft = nargs > 5 ? p[5] : 0.5;         // default is .5
         branch = skip;
      }

      float out[2];
      out[0] = in[i + inchan] * amp;

      if (outputChannels() == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      rtaddout(out);
      increment();
   }
   rtinrepos(this, -framesToRun(), SEEK_CUR);

   return framesToRun();
}


Instrument *makeREVMIX()
{
   REVMIX *inst;

   inst = new REVMIX();
   inst->set_bus_config("REVMIX");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("REVMIX", makeREVMIX);
}
#endif
