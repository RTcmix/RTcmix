/* MROOM - room simulation with moving source

   First call timeset at least twice to set trajectory of moving
   sound source:
      p0     time
      p1     x location
      p2     y location

   Then call MROOM:
      p0     output start time
      p1     input start time
      p2     input duration
      p3     amplitude multiplier
      p4     distance from middle of room to right wall (i.e., 1/2 of width)
      p5     distance from middle of room to front wall (i.e., 1/2 of depth)
      p6     reverb time (in seconds)
      p7     reflectivity (0 - 100; the higher, the more reflective)
      p8     "inner room" width (try 8)
      p9     input channel number   [optional]
      p10    control rate for trajectory   [optional]
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include "MROOM.h"
#include <rt.h>
#include <rtdefs.h>
#include <delmacros.h>

//#define DEBUG

#define INTERP_GET                    /* use interpolating delay line fetch */

#define DEFAULT_QUANTIZATION  100
#define MAX_DELAY             1.0     /* seconds */
#define AVERAGE_CHANS         -1      /* average input chans flag value */


MROOM::MROOM() : Instrument()
{
   in = NULL;
   delayline = rvbarrayl = rvbarrayr = NULL;
   branch = quantbranch = 0;
}


MROOM::~MROOM()
{
   delete [] in;
   delete [] delayline;
   delete [] rvbarrayl;
   delete [] rvbarrayr;
}


/* ----------------------------------------------------------------- init --- */
int MROOM::init(double p[], int n_args)
{
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   ovamp = p[3];
   xdim = p[4];
   ydim = p[5];
   float rvbtime = p[6];
   reflect = p[7];
   innerwidth = p[8];
   inchan = n_args > 9 ? (int)p[9] : AVERAGE_CHANS;
   int quant = n_args > 10 ? (int)p[10] : DEFAULT_QUANTIZATION;

   if (outputchans != 2)
      return die("MROOM", "Requires stereo output.");

   float ringdur = (rvbtime > MAX_DELAY) ? rvbtime : MAX_DELAY;
   if (rtsetoutput(outskip, dur + ringdur, this) == -1)
      return DONT_SCHEDULE;

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   insamps = (int)(dur * SR + 0.5);

   if (inchan >= inputChannels())
      return die("MROOM",
                 "You asked for channel %d of a %d-channel input file.",
                 inchan, inputChannels());
   if (inputChannels() == 1)
      inchan = 0;

// ***FIXME: input validation for trajectory points?
   int ntimes = get_timeset(timepts, xvals, yvals);
   if (ntimes == 0)
      return die("MROOM", "Must have at least two timeset calls before MROOM.");

   traject(ntimes);

   tableset(SR, dur, POS_ARRAY_SIZE, xpostabs);
   tableset(SR, dur, POS_ARRAY_SIZE, ypostabs);

   int delsamps = (int)(MAX_DELAY * SR + 0.5);
   delayline = new float[delsamps];
   delset(SR, delayline, deltabs, MAX_DELAY);

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
      rtcmix_advise("MROOM", "Setting phrase curve to all 1's.");
   aamp = ovamp;                  /* in case amparray == NULL */

   skip = (int)(SR / (float)resetval);
   quantskip = (int)(SR / (float)quant);

   return nSamps();
}


/* ------------------------------------------------------------ configure --- */
int MROOM::configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


/* ------------------------------------------------------------------ run --- */
int MROOM::run()
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
         if (--quantbranch <= 0) {
            float xposit = tablei(currentFrame(), xpos, xpostabs);
            float yposit = tablei(currentFrame(), ypos, ypostabs);
            distndelset(xposit, yposit, xdim, ydim, innerwidth, reflect);
            quantbranch = quantskip;
         }

         if (inchan == AVERAGE_CHANS) {
            insig = 0.0;
            for (int n = 0; n < inputChannels(); n++)
               insig += in[i + n];
            insig /= (float)inputChannels();
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
#ifdef INTERP_GET
         DLIGET(delayline, del[m], deltabs, delval);
#else
         DELGET(delayline, del[m], deltabs, delval);
#endif
         rout += delval * amp[m];
         if (m < 2)
            rvbsig = -rout;
      }
      rvbsig += rout;

      float out[2];
      out[0] = rout + reverb(rvbsig, rvbarrayr);

      float lout = 0.0;
      for (int m = 0; m < NTAPS; m += 2) {
#ifdef INTERP_GET
         DLIGET(delayline, del[m], deltabs, delval);
#else
         DELGET(delayline, del[m], deltabs, delval);
#endif
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


/* -------------------------------------------------------------- traject --- */
void MROOM::traject(int ntimes)
{
   int   i, n, m;
   float xincr, yincr, scaler;

   scaler = (float)POS_ARRAY_SIZE / timepts[--ntimes];

   n = m = 0;
   for (i = 0; i < ntimes; i++) {
      n += (int)((timepts[i + 1] - timepts[i]) * scaler);
      if (n > POS_ARRAY_SIZE - 1)
         n = POS_ARRAY_SIZE - 1;
      xincr = (xvals[i + 1] - xvals[i]) / (float)(n - m);
      yincr = (yvals[i + 1] - yvals[i]) / (float)(n - m);
      xpos[m] = xvals[i];
      ypos[m] = yvals[i];
      for ( ; m < n; m++) {
         xpos[m + 1] = xpos[m] + xincr;
         ypos[m + 1] = ypos[m] + yincr;
      }
   }

   while (m < POS_ARRAY_SIZE - 1) {
      xpos[m + 1] = xpos[m];
      ypos[m + 1] = ypos[m];
      m++;
   }
}


/* ---------------------------------------------------------- distndelset --- */
float MROOM::distndelset(float xsource,
                         float ysource,
                         float xouter,
                         float youter,
                         float inner,
                         float reflect)
{
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

   for (int m = 0; m < NTAPS; m++) {
      dist[m] = sqrt(dist[m]);
      del[m] = (float)(dist[m] / 1086.0);
      amp[m] = (float)((dist[m] - dist[0]) / dist[0] * -6.0);
      amp[m] = (float)pow(10.0, (double)amp[m] / 20.0);
      if (m > 2)
         amp[m] = amp[m] * reflect / 100.0;
   }

   if (!ysource) {
      if (xsource < inner)
         (xsource < -inner) ? (amp[0] = 0.0) : (amp[0] = amp[1] = 0.0);
      else
         amp[1] = 0.0;
   }

   return 0.0;
}


Instrument *makeMROOM()
{
   MROOM *inst;

   inst = new MROOM();
   inst->set_bus_config("MROOM");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("MROOM", makeMROOM);
}
#endif
