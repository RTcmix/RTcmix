#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Instrument.h>
#include "ROOM.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG
#define AVERAGE_CHANS   -1           /* average input chans flag value */


ROOM::ROOM() : Instrument()
{
   in = NULL;
   echo = NULL;
   branch = 0;
}


ROOM::~ROOM()
{
   delete [] in;
   delete [] echo;
}


int ROOM::init(double p[], int n_args)
{
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   amp = p[3];
   inchan = n_args > 4 ? (int)p[4] : AVERAGE_CHANS;

   if (outputchans != 2)
      return die("ROOM", "Output must be stereo.");

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   insamps = (int) (dur * SR + 0.5);

   if (inchan >= inputChannels())
      return die("ROOM", "You asked for channel %d of a %d-channel input file.",
                                                   inchan, inputChannels());
   if (inputChannels() == 1)
      inchan = 0;

   nmax = get_room(ipoint, lamp, ramp, SR);
   if (nmax == 0)
      return die("ROOM", "You need to call roomset before ROOM.");

   echo = new float[nmax];
   for (int i = 0; i < nmax; i++)
      echo[i] = 0.0;
   jpoint = 0;

#ifdef DEBUG
   printf("maximum delay = %d samples.\n", nmax);
#endif

   float ringdur = (float) nmax / SR;
   if (rtsetoutput(outskip, dur + ringdur, this) == -1)
      return DONT_SCHEDULE;

   amparray = floc(1);
   if (amparray) {
      int amplen = fsize(1);
      tableset(SR, dur + ringdur, amplen, amptabs);
   }
   else
      rtcmix_advise("ROOM", "Setting phrase curve to all 1's.");
   aamp = amp;                  /* in case amparray == NULL */

   skip = (int)(SR / (float)resetval);

   return nSamps();
}


int ROOM::configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int ROOM::run()
{
   const int samps = framesToRun() * inputChannels();

   rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         if (amparray)
            aamp = tablei(currentFrame(), amparray, amptabs) * amp;
         branch = skip;
      }

      float insig;
      if (currentFrame() < insamps) {        /* still taking input */
         if (inchan == AVERAGE_CHANS) {
            insig = 0.0;
            for (int n = 0; n < inputChannels(); n++)
               insig += in[i + n];
            insig /= (float) inputChannels();
         }
         else
            insig = in[i + inchan];
      }
      else                                   /* in ring-down phase */
         insig = 0.0;

      echo[jpoint++] = insig;
      if (jpoint >= nmax)
         jpoint -= nmax;

      float out[2];
      out[0] = out[1] = 0.0;
      for (int j = 0; j < NTAPS; j++) {
         float e = echo[ipoint[j]];
         out[0] += e * lamp[j];
         out[1] += e * ramp[j];
         ipoint[j]++;
         if (ipoint[j] >= nmax)
            ipoint[j] -= nmax;
      }

      if (aamp != 1.0) {
         out[0] *= aamp;
         out[1] *= aamp;
      }

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeROOM()
{
   ROOM *inst;

   inst = new ROOM();
   inst->set_bus_config("ROOM");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("ROOM", makeROOM);
}
#endif
