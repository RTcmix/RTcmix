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
#include <mixerr.h>
#include <Instrument.h>
#include "SROOM.h"
#include <rt.h>
#include <rtdefs.h>

extern "C" {
   #include <ugens.h>
   #include <delmacros.h>
   extern int resetval;
}

//#define DEBUG
#define AVERAGE_CHANS   -1           /* average input chans flag value */


SROOM::SROOM() : Instrument()
{
   delayline = rvbarrayl = rvbarrayr = NULL;
}


SROOM::~SROOM()
{
   delete [] delayline;
   delete [] rvbarrayl;
   delete [] rvbarrayr;
}


int SROOM::init(float p[], short n_args)
{
   int   rvbsamps;
   float outskip, inskip, dur;
   float xdim, ydim, xsrc, ysrc, rvbtime, reflect, innerwidth;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   ovamp = p[3];
   xdim = p[4];
   ydim = p[5];
   xsrc = p[6];
   ysrc = p[7];
   rvbtime = p[8];
   reflect = p[9];
   innerwidth = p[10];        /* the room inside your head */
   inchan = n_args > 11 ? (int)p[11] : AVERAGE_CHANS;

   if (NCHANS != 2) {
      fprintf(stderr, "SROOM requires stereo output.\n");
      exit(1);
   }

   nsamps = rtsetoutput(outskip, dur + rvbtime, this);
   rtsetinput(inskip, this);
   insamps = (int)(dur * SR);

   if (inchan >= inputchans) {
      fprintf(stderr, "You asked for channel %d of a %d-channel input file.\n",
                       inchan, inputchans);
      exit(1);
   }

   if (inputchans == 1)
      inchan = 0;

   distndelset(xsrc, ysrc, xdim, ydim, innerwidth, reflect);

   /* Array dimensions taken from lib/rvbset.c (+ 2 extra for caution). */
   rvbsamps = (int)((0.1583 * SR) + 18 + 2);
   rvbarrayl = new float[rvbsamps];
   rvbarrayr = new float[rvbsamps];
   rvbset(rvbtime, 0, rvbarrayl);
   rvbset(rvbtime, 0, rvbarrayr);

   amparray = floc(1);
   if (amparray) {
      int amplen = fsize(1);
      tableset(dur, amplen, amptabs);
   }
   else
      printf("Setting phrase curve to all 1's\n");

   skip = (int)(SR / (float)resetval);

   return nsamps;
}


int SROOM::run()
{
   int   i, m, branch, rsamps;
   float aamp, insig, lout, rout, delval, rvbsig = 0.0;
   float in[2 * MAXBUF], out[2];

   rsamps = chunksamps * inputchans;

   rtgetin(in, this, rsamps);

   aamp = ovamp;                  /* in case amparray == NULL */

   branch = 0;
   for (i = 0; i < rsamps; i += inputchans) {
      if (cursamp < insamps) {               /* still taking input from file */
         if (--branch < 0) {
            if (amparray)
               aamp = tablei(cursamp, amparray, amptabs) * ovamp;
            branch = skip;
         }
         if (inchan == AVERAGE_CHANS) {
            insig = 0.0;
            for (int n = 0; n < inputchans; n++)
               insig += in[i + n];
            insig /= (float)inputchans;
         }
         else
            insig = in[i + inchan];
         insig *= aamp;
      }
      else                                   /* in ring-down phase */
         insig = 0.0;

      DELPUT(insig, delayline, deltabs);

      rout = 0.0;
      for (m = 1; m < NTAPS; m += 2) {
         DELGET(delayline, del[m], deltabs, delval);
         rout += delval * amp[m];
         if (m < 2)
            rvbsig = -rout;
      }
      rvbsig += rout;
      out[0] = rout + reverb(rvbsig, rvbarrayr);

      lout = 0.;
      for (m = 0; m < NTAPS; m += 2) {
         DELGET(delayline, del[m], deltabs, delval);
         lout += delval * amp[m];
         if (m < 2)
            rvbsig = -lout;
      }
      rvbsig += lout;
      out[1] = lout + reverb(rvbsig, rvbarrayl);

      rtaddout(out);
      cursamp++;
   }

   return i;
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

   delset(delayline, deltabs, (float)nmax / SR);

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
   return inst;
}


void rtprofile()
{
   RT_INTRO("SROOM", makeSROOM);
}

