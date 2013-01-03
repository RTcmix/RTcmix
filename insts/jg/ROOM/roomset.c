#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ugens.h>
#include "roomset.h"

static int roomset_called = 0;
static float delay[NTAPS], sloc[NTAPS], amp[NTAPS];


/* ------------------------------------------------------------- get_room --- */
/* Fills in <ipoint>, <lamp> and <ramp> arrays, each dimensioned for NTAPS
   elements. Returns number of samples for maximum delay time. (Caller needs
   this to allocate an array for that many samples.)
   If roomset Minc function hasn't been called, returns 0.
*/
int
get_room(int ipoint[], float lamp[], float ramp[], double SR)
{
   int i, nmax;

   if (!roomset_called)
      return 0;

   nmax = 0;
   for (i = 0; i < NTAPS; i++) {
      int nsmps = delay[i] * SR + 0.5;
      nmax = (nsmps > nmax) ? nsmps : nmax;
   }

   for (i = 0; i < NTAPS; i++) {
      ipoint[i] = (nmax - (int)(delay[i] * SR) + 1) % (nmax + 1);
      lamp[i] = amp[i] * (1.0 - sloc[i]);
      ramp[i] = amp[i] * sloc[i];
      /* Note: in original version, left and right seemed swapped, so
         I fixed it here. Didn't look further, though.  -JGG  */
   }

   return nmax;
}


/* ---------------------------------------------------------------- specs --- */
static int
specs(float source[],     /* first 3 args: arrays of 2 elements */
      float bounce[],
      float xlist[],
      float dec,
      float *adelay,      /* last 3 args passed back to caller */
      float *ansloc,
      float *anamp)
{
   int    i;
   float  dist0[2], dist1[2], dist2[2];
   double w, x, y, z, d0, d1, d2, fact, dist, angle;

   fact = 1.0 / PI;

   w = x = y = z = 0.0;

   for (i = 0; i < 2; i++) {
      dist0[i] = -xlist[i];
      dist1[i] = bounce[i] - source[i];
      dist2[i] = bounce[i] - xlist[i];
      w += (double)(dist0[i] * dist0[i]);
      x += (double)(dist1[i] * dist1[i]);
      y += (double)(dist2[i] * dist2[i]);
      z += (double)(dist0[i] * dist2[i]);
   }

   d0 = sqrt(w);
   d1 = sqrt(x);
   d2 = sqrt(y);
   x = z / (d0 * d2);
   if (x > 1.0)
      x = 1.0;
   angle = acos(x);
   dist = d1 + d2;

   *ansloc = (float)(angle / PI);
   *adelay = (float)(dist / 1087.0);       /* speed of sound */
   *anamp = (float)((pow(2.0, (double)dec) / pow(dist, (double)dec)) * 100.0);

   return 0;
}


/* --------------------------------------------------------------- rindom --- */
static float
rindom(float *a)
{
   long n = (long)(*a * 1048576.0);
   return (float)((1061L * n + 221589L) % 1048576L) / 1048576.0;
}


/* ----------------------------------------------------------------- rind --- */
static float
rind(float amp, float *a)
{
   *a = rindom(a);
   return (amp * (1. - 2. * *a));
}


/* ---------------------------------------------------------------- space --- */
static int
space(float dim[],          /* first 4 args: arrays of 2 elements */
      float source[],
      float warp1[],
      float warp2[],
      float dec,
      float rndval)
{
   int    i;
   double pow1, pow2, pow3, pow4, rval, xval, x;
   float  rndseed, bounce[NTAPS][2];
   float  xxx, xlist[2];

   rndseed = (rndval == 0.0) ? 0.3 : rndval;

   pow1 = log((double)warp1[0]) / log(0.5);
   pow2 = log((double)warp1[1]) / log(0.5);
   pow3 = log((double)warp2[0]) / log(0.5);
   pow4 = log((double)warp2[1]) / log(0.5);

   for (i = 0; i < 2; i++) {
      source[i] *= dim[i];
      x = (double)i / 2.0;

      rval = ((double)rind(0.5, &rndseed) + 0.5) / 2.0 + x;
      xval = exp(log(rval) / pow1);
      bounce[i][0] = (float)rval;
      bounce[i][1] = (float)pow(xval, pow2);

      rval = ((double)rind(0.5, &rndseed) + 0.5) / 2.0 + x;
      xval = exp(log(rval) / pow3);
      bounce[i + 4][0] = (float)rval;
      bounce[i + 4][1] = (float)pow(xval, pow4);

      rval = ((double)rind(0.5, &rndseed) + 0.5) / 2.0 + x;
      xval = exp(log(rval) / pow2);
      bounce[i + 2][1] = (float)rval;
      bounce[i + 2][0] = (float)pow(xval, pow1);

      rval = ((double)rind(0.5, &rndseed) + 0.5) / 2.0 + x;
      xval = exp(log(rval) / pow4);
      bounce[i + 6][1] = (float)rval;
      bounce[i + 6][0] = (float)pow(xval, pow3);

      bounce[i + 8][0] = (rind(0.5, &rndseed) + 0.5) / 2.0 + x;
      bounce[i + 10][0] = (rind(0.5, &rndseed) + 0.5) / 2.0 + x;
      bounce[i + 8][1] = bounce[i + 10][1] = 0.0;

      bounce[NTAPS - 1][i] = source[i];
   }

   for (i = 0; i < 4; i++) {
      bounce[i][0] *= (0.5 * dim[0]);
      bounce[i][1] *= dim[1];
      bounce[i + 4][0] = (1.0 - bounce[i + 4][0] * 0.5) * dim[0];
      bounce[i + 4][1] *= dim[1];
   }
   for (i = 8; i < 10; i++) {
      bounce[i][0] *= 0.5 * dim[0];
      bounce[i + 2][0] = (1.0 - bounce[i + 2][0] * 0.5) * dim[0];
   }

   xlist[0] = 0.5 * dim[0];
   xlist[1] = 0.0;
   bounce[NTAPS - 1][0] = source[0];
   bounce[NTAPS - 1][1] = source[1];

   xxx = 0;
   for (i = 0; i < NTAPS; i++) {
      float d, s, a;
      specs(source, &bounce[i][0], xlist, dec, &d, &s, &a);
      delay[i] = d;
      sloc[i] = s;
      amp[i] = a * boost(s);
      xxx += amp[i];
   }
   printf(" x-loc   y-loc   delay    sloc     amp\n");
   for (i = 0; i < NTAPS; i++) {
      amp[i] /= xxx;
      delay[i] -= delay[NTAPS - 1];
      printf("%7.2f %7.2f  %6.5f  %6.5f  %6.5f\n",
             bounce[i][0], bounce[i][1], delay[i], sloc[i], amp[i]);
   }

   return 0;
}


/* -------------------------------------------------------------- roomset --- */
/* Minc function to set up ROOM params.
      p0 = x wall length
      p1 = y wall length
      p2 = x source position
      p3 = y source position
      p4 = x wall left            [see README for p4-p7]
      p5 = y wall left
      p6 = x wall right
      p7 = y wall right
      p8 = absorption factor (the higher, the more absorption)
      p9 = seed  [optional]
*/
double
roomset(float p[], int n_args)
{
   if (n_args < 9)
      die("roomset", "Not enough args.");

// ***FIXME: need some input validation here

   /* NOTE: 1st 4 args interpreted as arrays with 2 elements */
   space(&p[0], &p[2], &p[4], &p[6], p[8], p[9]);

   roomset_called = 1;

   return 0.0;
}

#ifndef MAXMSP
/* -------------------------------------------------------------- profile --- */
int
profile()
{
   UG_INTRO("roomset", roomset);
   return 0;
}
#endif

