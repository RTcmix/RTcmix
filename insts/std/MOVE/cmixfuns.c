#include <math.h>
#include <ugens.h>
#include "cmixfuns.h"


/* ---------------------------------------------------------------- rand1 --- */
/* rand1 returns floating point values between 0 and 1 */
inline double
rand1(double *x)
{
   register int n = *x * 1048576.0;
   *x = (double)((1061 * n + 221589) % 1048576) / 1048576.0;
   return (*x);
}


/* ---------------------------------------------------------------- randi --- */
/* randi produces interpolated linear ramps between successive random
   numbers at a rate determined in a setup routine.

   a[0] is the amp.  a[1] is the SI.  a[2] is set to 1.0 externally.
   a[4] is the seed, set externally.
*/
double
randi(double a[6])
{
   /* advance counter by fraction of 512 (SI) */
   a[2] += a[1] / 512.0;

   /* every time counter exceeds 1.0, calculate new rand # */
   if (a[2] >= 1.0) {
      a[2] -= 1.0;              /* reset counter */
      a[3] = a[4];              /* store old value */
      a[4] = rand1(a + 4);      /* get new rand val */
      a[5] = 2.0 * (a[3] - a[4]);  /* twice the difference */
      a[3] = 1.0 - (2.0 * a[3]) - (a[5] * a[2]);
   }

   /* output is linear interpolation between old and new, centered around
      zero, and multiplied by the amp factor. */

   return ((a[3] + a[5] * a[2]) * a[0]);
}

#if 0	/* using inline version in PLACE.C */

/* ----------------------------------------------------------------- tone --- */
/* tone is a simple 1st order recursive lowpass filter
*/
double
tone(double sig, double data[3])
{
   data[2] = data[0] * sig + data[1] * data[2];
   return (data[2]);
}

#endif

/* -------------------------------------------------------------- toneset --- */
/* toneset calculates the coefficients for tone.
   cutoff is -3db point in cps.  flag will reset history if set to 1.
*/
void
toneset(float SR, double cutoff, int flag, double *data)
{
   double x = 2.0 - cos(cutoff * PI2 / SR);     /* feedback coeff. */
   data[1] = x - sqrt(x * x - 1.0);
   data[0] = 1.0 - data[1];                     /* gain coeff. */
   if (cutoff < 0.0)
      data[1] *= -1.0;                          /* inverse function */
   if (flag)
      data[2] = 0.0;
}

