/* Derived from gen4 from the UCSD Carl package, described in F.R. Moore, 
   "Elements of Computer Music."    -JGG, 12/2/01
*/
#include <math.h>
#include <ugens.h>


/* ---------------------------------------------------------------- trans --- */
/* trans(a, alpha, b, n, output) makes a transition from <a> to <b> in
   <n> steps, according to transition parameter <alpha>.  It stores the
   resulting <n> values starting at location <output>.
      alpha = 0 yields a straight line,
      alpha < 0 yields an exponential transition, and 
      alpha > 0 yields a logarithmic transition.
   All of this in accord with the formula:
      output[i] = a + (b - a) * (1 - exp(i * alpha / (n-1))) / (1 - exp(alpha))
   for 0 <= i < n
*/
static void
trans(float a, float alpha, float b, int n, float *output)
{
   int   i;
   float d, e, interval, delta;

   delta = b - a;

   if (n > 1)
      interval = 1.0 / (n - 1.0);
   else
      die("gen4", "transition length <= 1"); //FIXME: an internal error?

   if (alpha != 0.0) {
      float denom = 1.0 / (1.0 - exp((double) alpha));
      for (i = 0; i < n; i++)
         *output++ = a + delta * (1.0 - exp((double) i * alpha * interval))
                                                                     * denom;
   }
   else
      for (i = 0; i < n; i++)
         *output++ = a + delta * i * interval;
}


/* ----------------------------------------------------------------- gen4 --- */
#define MAX_POINTS 1024

double
gen4(struct gen *gen)
{
   int   i, points, seglen = 0;
   float factor, *ptr;
   float time[MAX_POINTS], value[MAX_POINTS], alpha[MAX_POINTS];

   if (gen->nargs < 5 || (gen->nargs % 3) != 2)    /* check number of args */
      die("gen4", "usage: t1 v1 a1 ... tn vn");

   if ((gen->nargs / 3) + 1 > MAX_POINTS)
      die("gen4", "too many arguments");

   for (i = points = 0; i < gen->nargs; points++) {
      time[points] = gen->pvals[i++];
      if (points > 0 && time[points] < time[points - 1])
         die("gen4", "non-increasing time values");
      value[points] = gen->pvals[i++];
      if (i < gen->nargs)
         alpha[points] = gen->pvals[i++];
   }

// what's this for?
   factor = (float) (gen->size - 1) / time[points - 1];
   for (i = 0; i < points; i++)
      time[i] *= factor;

   ptr = gen->array;
   for (i = 0; i < points - 1; i++) {
      seglen = floor(time[i + 1] + 0.5) - floor(time[i] + 0.5) + 1;
      trans(value[i], alpha[i], value[i + 1], seglen, ptr);
      ptr += seglen - 1;
   }

   fnscl(gen);

   return 0.0;
}


