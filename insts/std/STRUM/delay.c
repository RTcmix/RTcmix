#define maxdl 14000
#include <math.h>
#include <ugens.h>
#include "delayq.h"

float delay(float xin, delayq *q) 
 /* simple interpolating delay line, uses structure as set up by delayset */
{
   int pout,pout1;
   if( ++q->p >= maxdl) q->p = 0;
   pout = q->p - q->del;
   if ( pout < 0 )  pout = pout + maxdl;
   pout1 = pout-1;
   if ( pout1 < 0 )  pout = pout + maxdl;

   q->d[q->p] = xin;
   return(q->c1*q->d[pout] + q->c2*q->d[pout1]);
}
