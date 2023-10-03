#include <stdio.h>
#include <ugens.h>
#include "elldefs.h"
#include "setell.h"

#define VERBOSE 0      /* print stuff during filter design (in setell) */

double ellset(double p[], int n_args);
int get_nsections(void);
int ellpset(EllSect [], float *);
float ellipse(float, int, EllSect [], float);
extern float SR(); // hack to access what was global


// ***FIXME: Where did 461 come from?
//    Seems like there only needs to be 4 * MAX_SECTIONS plus 1 for xnorm.
static double coeffs[461];              /* array for coefficients */
static int    nsections = 0;            /* number of sections */


double
ellset(double p[], int n_args)
{
   double srate, f1, f2, f3, ripple, atten;

   f1 = (double)p[0];
   f2 = (double)p[1];
   f3 = (double)p[2];
   ripple = (double)p[3];
   atten = (double)p[4];
   srate = (double)SR();

// ***FIXME: do some input validation here

   setell(srate, f1, f2, f3, ripple, atten, coeffs, &nsections, VERBOSE);

   if (nsections < 1 || nsections > MAX_SECTIONS)
      die("ELL", "Filter design failed! Try relaxing specs.");

   return 0.0;
}


/* Returns number of filter sections (per channel).
   If zero, means user hasn't called ellset from Minc.
*/
int
get_nsections()
{
   return nsections;
}


int
ellpset(EllSect es[], float *xnorm)
{
   int     i, j;
   EllSect *s;

   for (j = i = 0; i < nsections; i++) {
      s = &es[i];
      s->c0 = (float)coeffs[j++];
      s->c1 = (float)coeffs[j++];
      s->c2 = (float)coeffs[j++];
      s->c3 = (float)coeffs[j++];
      s->x1 = s->x2 = s->y1 = s->y2 = 0.0;
   }
   *xnorm = (float)coeffs[j];

   return 0;
}


float
ellipse(float sig, int nsects, EllSect es[], float xnorm)
{  
   register int     i;
   register float   y0;
   register EllSect *s;

   for (i = 0; i < nsects; i++) {
      s = &es[i];

      y0 = sig + s->c0 * s->x1 + s->c2 * s->x2
               - s->c1 * s->y1 - s->c3 * s->y2;

      s->x2 = s->x1;
      s->x1 = sig;
      s->y2 = s->y1;
      s->y1 = y0;
      sig = y0;
   }
   return (sig * xnorm);
}

#ifndef EMBEDDED
int
profile()
{
	UG_INTRO("ellset", ellset);
   return 0;
}
#endif
