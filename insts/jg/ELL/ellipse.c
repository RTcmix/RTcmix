#include <stdio.h>
#include <defs.h>
#include <ugens.h>
#include "elldefs.h"

extern int setell_(float *, float *, float *, float *, float *,
                   float *, float *, int *);

double ellset(float p[], int n_args);
int get_nsections(void);
int ellpset(EllSect [], float *);
float ellipse(float, int, EllSect [], float);

// ***FIXME: Where did 461 come from?
//    Seems like there only needs to be 4 * MAX_SECTIONS plus 1 for xnorm.
static float coeffs[461];              /* array for coefficients */
static int   nsections = 0;            /* number of sections */


double
ellset(float p[], int n_args)
{
   float srate, f1, f2, f3, ripple, atten;

   f1 = p[0];
   f2 = p[1];
   f3 = p[2];
   ripple = p[3];
   atten = p[4];
   srate = SR;

// ***FIXME: do some input validation here

   setell_(&srate, &f1, &f2, &f3, &ripple, &atten, coeffs, &nsections);

   if (nsections < 1 || nsections > MAX_SECTIONS) {
      fprintf(stderr, "\nFilter design failed! Try relaxing specs.\n\n");
      exit(1);
   }

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
      s->c0 = coeffs[j++];
      s->c1 = coeffs[j++];
      s->c2 = coeffs[j++];
      s->c3 = coeffs[j++];
      s->x1 = s->x2 = s->y1 = s->y2 = 0.0;
   }
   *xnorm = coeffs[j];

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


int
profile()
{
	UG_INTRO("ellset", ellset);
   return 0;
}

