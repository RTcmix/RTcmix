#include <stdio.h>
#include <math.h>
#include <ugens.h>
#include "delayq.h"

void delayset(float SR, float freq, delayq *q) 
 /* Sets up structure for simple interpolating delay line, delay.c.
    Sets delay time to 1/freq.
    Uses linear interpolation to get better than 1 sample resolution of delay
       time. 
    Only sets parameters in structure, does not clean or initialize delayline.
    Call delayclean to do that.
      */

{
   float xdel,xerr;

   q->p = 0;
   xdel = 1./freq*SR;

   q->del = floor(xdel);
   if (q->del > maxdl) {
       q->del = maxdl;
       die("STRUM", "Too long a delay requested.");
       }
   xerr = xdel - q->del;

   /* Calculate averaging coefficients c1,c2 to do linear interpolation to 
   adjust the delay.  Has unfortunate consequence of also altering HF response*/ 
   q->c2 = xerr;
   q->c1 = 1. - xerr;
}
