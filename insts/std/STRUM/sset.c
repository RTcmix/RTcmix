#include <stdio.h>
#include <math.h>
#include <ugens.h>
#include "strums.h"

void sset(float SR, float freq, float tf0, float tNy, strumq *q) 

/* Sets up strumq structure for strum to use as plucked string.
   Uses a two point averaging filter to adjust the phase for exact
   correct pitch, and then uses a linear phase three point averaging
   filter to adjust the fundamental frequency to decay in time tf0,
   and in time tNy at the Nyquist frequency.  In some cases, the 
   decay time at the Nyquist frequency (which always must be less than 
   tf0) will not be exactly as requested, but a wide range of variation
   is possible.  The two point and three point filters are combined into
   a single four point filter.  A single pole dc-blocking filter is added
   because the four point filter may have some gain a dc, and non-zero dc
   response can cause problems with clicks and what not.     
   Randfill must be called after (and not before) this routine is called the 
   first time, as it initializes some things in the strumq structure.
   This routine does not initialize them so it may be used to change parameters
   during the course of a note.

                    Charlie Sullivan
                    1/87                                      */

{
   float xlen,xn,xerr,dH0,dHNy,H01,H02,HNy1,HNy2,H,a0,a1,c1,c2;
   float ncycles0,ncyclesNy;
   float tgent,c,s,del,p,temp,g;
   double w0;
   int i;

   xlen = 1./freq*SR;
   w0 = freq/SR*2.*PI;

   ncycles0 = freq * tf0;
   ncyclesNy = freq * tNy;
   /*ncylces is not an integer,and is number of cycles to decay*/
   dH0 = pow(.1,(double)1./ncycles0); /* level will be down to -20db after t*/
   dHNy = pow(.1,(double)1./ncyclesNy); 

   del = 1.;  /*delay of 1 from three point filter to be added later */
   q->n = floor(xlen - del);
   if (q->n > maxlen) {
      rtcmix_warn("STRUM", "Pitch is too low.");
      q->n = maxlen;
   }

   xerr = q->n - xlen + del;   /*xerr will be a negative number*/

   /* Calculate the phase shift needed from two-point averaging filter,
      calculate the
      filter coefficient c1:  y = c1*xn + (1-c1)*x(n-1)  */
   tgent = tan((double)xerr*w0);  /* tan of theta */
   c = cos(w0);
   s = sin(w0);
   c1 = (-s-c*tgent)/(tgent*(1.-c)-s);
   c2 = 1. - c1;

   /*effect of this filter on amplitude response*/
   H01 = sqrt( (c2)*(c2)*s*s + (c1*(1.-c)+c)*(c1*(1.-c)+c) );
   HNy1 = fabs(2.*c1 - 1.);

   /*Now add three point linear phase averaging filter with delay of 1, */
   /* y = xn*a0 + xn-1*a1 + xn-2*a0  */
   /* and a gain or loss factor, g, so that the filter*g has repsonse H02 and*/
   /* HNy2 to make the total response of all the filters dH0 and dHNy  */
   H02 = dH0/H01;
   if (HNy1 >0) HNy2 = dHNy/HNy1;
   else {
      HNy2 = 1.e10;
      /*printf("unable to meet specs exactly--you requested a magic frequency\n");*/
   }
   /*printf("dHNy=%f,HNy2=%f,HNy1=%f\n",dHNy,HNy2,HNy1);*/
   g = (2*H02 - (1.-c)*HNy2)/(1.+c);
   a1 = (HNy2/g + 1.)/2.;
   /*a1 = (H02/g - c)/(1.-c);*/ /*alternate equivalent expression*/

   /*However, for this filter to be monotonic low pass, a1 must be between */
   /* 1/2 and 1, if it isn't response at Nyquist won't be as specified, */
   /*but it will be set as close as is feasible*/
   if( a1<.5) {
     a1 = .5;
     H = (1.-a1)*c + a1;
     g = H02/H;
     /*printf("specs won't be met exactly, too fast a Nyquist freq. decay was requested.\n");*/
   }
   
   if( a1>1.) {
     a1 = 1.;
     g = H02;
     /*printf("specs won't be met exactly, too slow a Nyquist decay was requested\n");*/
   }

   a0 = (1.-a1)/2.;
   a0 *= g;
   a1 *= g;


   /* Now combine the two and three point averaging filters into one */
   /* four point filter with coefficients a[0]-a[3] */
   q->a[0] = a0*c1;
   q->a[1] = a0*c2 + a1*c1;
   q->a[2] = a0*c1 + a1*c2;
   q->a[3] = a0*c2;

   /*set up dc blocking filter*/
   temp=PI*(float)(freq/18./SR);  /*cutoff frequency at freq/18 */
   q->dca0=1./(1.+temp);
   q->dca1= -q->dca0;
   q->dcb1=q->dca0*(1-temp);


}
