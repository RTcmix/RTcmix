/* SROOM - room simulation with stationary sound source
      p0     output start time
      p1     input start time
      p2     input duration
      p3     amplitude multiplier
      p4     distance from middle of room to right wall (i.e., 1/2 of width)
      p5     distance from middle of room to front wall (i.e., 1/2 of depth)
      p6,p7  x,y position of source  (middle of room is 0,0)
      p8     reverb time (in seconds)
      p9     reflectivity (0 - 100; the higher, the more reflective)
      p10    "inner room" width (try 8)
      p11    input channel number   [optional]
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include "SROOM.h"
#include <rt.h>
#include <rtdefs.h>
#include <delmacros.h>

//#define DEBUG
#define AVERAGE_CHANS   -1           /* average input chans flag value */


SROOM::SROOM() : Instrument()
{
   in = NULL;
   delayline = rvbarrayl = rvbarrayr = NULL;
   branch = 0;
}


SROOM::~SROOM()
{
   delete [] in;
   delete [] delayline;
   delete [] rvbarrayl;
   delete [] rvbarrayr;
}


int SROOM::init(double p[], int n_args)
{
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   ovamp = p[3];
   float xdim = p[4];
   float ydim = p[5];
   float xsrc = p[6];
   float ysrc = p[7];
   float rvbtime = p[8];
   float reflect = p[9];
   float innerwidth = p[10];        /* the room inside your head */
   inchan = n_args > 11 ? (int)p[11] : AVERAGE_CHANS;

   if (outputChannels() != 2)
      return die("SROOM", "Output must be stereo.");

   if (rtsetoutput(outskip, dur + rvbtime, this) == -1)
      return DONT_SCHEDULE;
   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   insamps = (int) (dur * SR + 0.5);

   if (inchan >= inputChannels())
      return die("SROOM",
                 "You asked for channel %d of a %d-channel input file.",
                 inchan, inputChannels());
   if (inputChannels() == 1)
      inchan = 0;

   distndelset(xsrc, ysrc, xdim, ydim, innerwidth, reflect);

   /* Array dimensions taken from lib/rvbset.c (+ 2 extra for caution). */
   int rvbsamps = (int)((0.1583 * SR) + 18 + 2);
   rvbarrayl = new float[rvbsamps];
   rvbarrayr = new float[rvbsamps];
   rvbset(SR, rvbtime, 0, rvbarrayl);
   rvbset(SR, rvbtime, 0, rvbarrayr);

   amparray = floc(1);
   if (amparray) {
      int amplen = fsize(1);
      tableset(SR, dur, amplen, amptabs);
   }
   else
      rtcmix_advise("SROOM", "Setting phrase curve to all 1's.");
   aamp = ovamp;                  /* in case amparray == NULL */

   skip = (int)(SR / (float)resetval);

   return nSamps();
}


int SROOM::configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int SROOM::run()
{
   const int samps = framesToRun() * inputChannels();

   rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      float insig, delval = 0.0, rvbsig = 0.0;

      if (currentFrame() < insamps) {        /* still taking input */
         if (--branch <= 0) {
            if (amparray)
               aamp = tablei(currentFrame(), amparray, amptabs) * ovamp;
            branch = skip;
         }
         if (inchan == AVERAGE_CHANS) {
            insig = 0.0;
            for (int n = 0; n < inputChannels(); n++)
               insig += in[i + n];
            insig /= (float) inputChannels();
         }
         else
            insig = in[i + inchan];
         insig *= aamp;
      }
      else                                   /* in ring-down phase */
         insig = 0.0;

      DELPUT(insig, delayline, deltabs);

      float rout = 0.0;
      for (int m = 1; m < NTAPS; m += 2) {
         DELGET(delayline, del[m], deltabs, delval);
         rout += delval * amp[m];
         if (m < 2)
            rvbsig = -rout;
      }
      rvbsig += rout;

      float out[2];
      out[0] = rout + reverb(rvbsig, rvbarrayr);

      float lout = 0.0;
      for (int m = 0; m < NTAPS; m += 2) {
         DELGET(delayline, del[m], deltabs, delval);
         lout += delval * amp[m];
         if (m < 2)
            rvbsig = -lout;
      }
      rvbsig += lout;
      out[1] = lout + reverb(rvbsig, rvbarrayl);

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


/* ---------------------------------------------------------- distndelset --- */
float SROOM::distndelset(float xsource,
                         float ysource,
                         float xouter,
                         float youter,
                         float inner,
                         float reflect)
{
   int    m, nmax, nsmps;
   double pow1, pow2, pow3, pow4, pow5, pow6, pow7, pow8, pow9;
   double dist[NTAPS];

   pow1 = pow((double)(xsource - inner), 2.0);
   pow2 = pow((double)(xsource + inner), 2.0);
   pow3 = pow((double)(ysource - 1.0), 2.0);
   pow4 = pow((double)((ysource - 1.0) + 2.0 * (youter - ysource)), 2.0);
   pow5 = pow((double)((ysource - 1.0) - 2.0 * (youter + ysource)), 2.0);
   pow6 = pow((double)((xsource - inner) + 2.0 * (xouter - xsource)), 2.0);
   pow7 = pow((double)((xsource + inner) + 2.0 * (xouter - xsource)), 2.0);
   pow8 = pow((double)((xsource - inner) - 2.0 * (xouter + xsource)), 2.0);
   pow9 = pow((double)((xsource + inner) - 2.0 * (xouter + xsource)), 2.0);

   dist[0] = pow1 + pow3;
   dist[1] = pow2 + pow3;
   dist[2] = pow1 + pow4;
   dist[3] = pow2 + pow4;
   dist[4] = pow6 + pow3;
   dist[5] = pow7 + pow3;
   dist[6] = pow1 + pow5;
   dist[7] = pow2 + pow5;
   dist[8] = pow8 + pow3;
   dist[9] = pow9 + pow3;

   nmax = 0;
   for (m = 0; m < NTAPS; m++) {
      dist[m] = sqrt(dist[m]);
      del[m] = (float)(dist[m] / 1086.0);
      amp[m] = (float)((dist[m] - dist[0]) / dist[0] * -6.0);
      amp[m] = (float)pow(10.0, (double)amp[m] / 20.0);
      nsmps = (int)(del[m] * SR + 0.5);
      nmax = (nsmps > nmax) ? nsmps : nmax;
      if (m > 2)
         amp[m] = amp[m] * reflect / 100.0;
   }

   delayline = new float[nmax];

   delset(SR, delayline, deltabs, (float)nmax / SR);

   if (!ysource) {
      if (xsource < inner)
         (xsource < -inner) ? (amp[0] = 0.0) : (amp[0] = amp[1] = 0.0);
      else
         amp[1] = 0.0;
   }

   return 0.0;
}


Instrument *makeSROOM()
{
   SROOM *inst;

   inst = new SROOM();
   inst->set_bus_config("SROOM");

   return inst;
}

void rtprofile()
{
   RT_INTRO("SROOM", makeSROOM);
}

