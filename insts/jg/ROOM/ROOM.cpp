#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
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
}


ROOM::~ROOM()
{
   delete [] in;
   delete [] echo;
}


int ROOM::init(float p[], int n_args)
{
   int   i, rvin;
   float outskip, inskip, dur, ringdur;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   inchan = n_args > 4 ? (int)p[4] : AVERAGE_CHANS;

   if (outputchans != 2) {
      die("ROOM", "Output must be stereo.");
		return(DONT_SCHEDULE);
	}

   rvin = rtsetinput(inskip, this);
	if (rvin == -1) { // no input
		return(DONT_SCHEDULE);
	}
   insamps = (int)(dur * SR);

   if (inchan >= inputchans) {
      die("ROOM", "You asked for channel %d of a %d-channel input file.",
                                                      inchan, inputchans);
		return(DONT_SCHEDULE);
	}
   if (inputchans == 1)
      inchan = 0;

   nmax = get_room(ipoint, lamp, ramp);
   if (nmax == 0) {
      die("ROOM", "You need to call roomset before ROOM.");
		return(DONT_SCHEDULE);
	}

   echo = new float[nmax];
   for (i = 0; i < nmax; i++)
      echo[i] = 0.0;
   jpoint = 0;

#ifdef DEBUG
   printf("maximum delay = %d samples.\n", nmax);
#endif

   ringdur = (float)nmax / SR;
   nsamps = rtsetoutput(outskip, dur + ringdur, this);

   amparray = floc(1);
   if (amparray) {
      int amplen = fsize(1);
      tableset(dur + ringdur, amplen, amptabs);
   }
   else
      advise("ROOM", "Setting phrase curve to all 1's.");

   skip = (int)(SR / (float)resetval);

   return nsamps;
}


int ROOM::run()
{
   int   i, branch, rsamps;
   float aamp, insig;
   float out[2];

   if (in == NULL)              /* first time, so allocate it */
      in = new float [RTBUFSAMPS * inputchans];

   Instrument::run();

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
      if (cursamp < insamps) {               /* still taking input from file */
         if (inchan == AVERAGE_CHANS) {
            insig = 0.0;
            for (int n = 0; n < inputchans; n++)
               insig += in[i + n];
            insig /= (float)inputchans;
         }
         else
            insig = in[i + inchan];
      }
      else                                   /* in ring-down phase */
         insig = 0.0;

      echo[jpoint++] = insig;
      if (jpoint >= nmax)
         jpoint -= nmax;

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
      cursamp++;
   }

   return i;
}


Instrument *makeROOM()
{
   ROOM *inst;

   inst = new ROOM();
   inst->set_bus_config("ROOM");

   return inst;
}

void rtprofile()
{
   RT_INTRO("ROOM", makeROOM);
}

