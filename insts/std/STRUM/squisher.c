#include <math.h>
#include <ugens.h>
#include "strums.h"

void
squisher(squish,q) 
 strumq *q;
 int squish;
{
   int i,j,p1,p2;
   float mult;
   double fabs(),cos();

/* Routine for use with 'strum' plucked string.  Called by randfill */
/* Low- pass filters vales of string, squish times. */
/* Compensates for loss of level at fundamental, but not for overall loss. */

p1 = q->n-1;
p2 = q->n-2;

mult = fabs(1./( 2.*cos((double)2.*PI/q->n) + 1.));

for(j=0; j<squish; j++){
    
   for(i=0;i<q->n;i++) {
       q->d[i] = mult*(q->d[p2]+q->d[i]+q->d[p1]);
       p2 = p1;
       p1 = i;
       }
 }

}
