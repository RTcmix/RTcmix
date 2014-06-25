#include <math.h>
#include <ugens.h>
#include "strums.h"

extern void squisher(int squish, strumq *q);


void randfill(float amp, int squish, strumq *q) 

/* Fills plucked string structure q with random values, and intitialize things.
   Call only after a call to sset.
   Can be used with zero amplitude to just initialize things.
   Squish models the softness of a plucking implement by filtering
   the values put in the string with an averaging filter.
   The filter makes squish passes.  The loss of amplitude at the fundamental
   frequency is compensated for, but the overall amplitude of the squished
   string is lowered, as the energy at other frequencies is decreased.  */

{
   float total,average;
   int i;

   q->p = q->n;

   q->dcz1=0;

/*set all values of array to zero*/
   for(i=0;i<maxlen;i++) {
       q->d[i]=0.;
       }

   /*printf("\n");
   for (i=1;i<100;i++) printf("%f\n",q->d[i]);*/

/* fill with white noise and subtract any dc component */
   total = 0.;
   for(i=0;i<q->n;i++) {
       q->d[i]=rrand()*amp;
       total=total+q->d[i];
       }

   /*printf("\n");
   for (i=1;i<100;i++) printf("%f\n",q->d[i]);*/

   average=total/((float)q->n);

   for(i=0;i<q->n;i++) {
       q->d[i]= q->d[i] - average;
       }

   squisher(squish,q);

}
