#include <math.h>
#include <ugens.h>
#include "strums.h"

float strum(float xin, strumq *q) 

/* Plucked string--uses a four point averaging filter (computed by sset)
   and a single pole dc blocking filter */

{
   float x,y;
   int p1,p2,p3,p4;

   if( ++q->p >= maxlen) q->p = 0;
   p1 = q->p - q->n;
   if(p1 < 0) p1 = p1 + maxlen;
   p2 = p1 - 1;
   if(p2 < 0) p2 = p2 + maxlen;
   p3 = p2 - 1;
   if(p3 < 0) p3 = p3 + maxlen;
   p4 = p3 - 1;
   if(p4 < 0) p4 = p4 + maxlen;


   /* averaging filter */
   x = q->a[3]*q->d[p4];
   x += q->a[2]*q->d[p3];
   x += q->a[1]*q->d[p2];
   x += q->a[0]*q->d[p1];


   /* dc blocking filter, for which xin+x is input, q->d[q->p] output */
   q->d[q->p]  = q->dca1 * q->dcz1;
   q->dcz1     = q->dcb1 * q->dcz1 + xin + x;
   q->d[q->p] += q->dca0 * q->dcz1;
   return(q->d[q->p]);
}

