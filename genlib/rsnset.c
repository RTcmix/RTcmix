#include <math.h>
#include "../H/ugens.h"
#include "../H/dbug.h"

void rsnset(float cf, float bw, float scl, float xinit, float a[])
{
	float c,temp;
	float tval1,tval2,tval3,tval4;

	c=temp=tval1=tval2=tval3=tval4=0;

	if(!xinit) {
		a[4] = 0;
		a[3] = 0;
		}

	a[2] = (float)exp(-PI2 * bw/SR);
	temp = 1. - a[2];
	c = a[2] + 1;
	a[1] = 4. * a[2]/c * (float)cos(PI2 * cf/SR);
	if(scl < 0) a[0] = 1;
	/* DJT:  Added this to workaround strange NaN values cropping up with IIR */
	/* Basically just broke up the math ... */
	/* My guess is that somewhere a fn() is being defined as float, but being */
	/* called or prototyped as type int ... causing an FP register corruption */
	/* I couldn't find it though */
	/* It's very strange because the disk-time iir works fine with the same code!*/

	if(scl) a[0] = sqrt(temp/c*(c*c-a[1]*a[1]));
	if(!scl) a[0] = temp*sqrt(1.-a[1]*a[1]/(4.*a[2]));

/* 	if (scl) { */
/* 	  tval1 = (c*c)-(a[1]*a[1]); */
/* 	  tval2 = (temp/c); */
/* 	  tval3 = tval1 * tval2; */
/* 	  tval4 = sqrt(tval3); */
/* 	  a[0] = tval4; */
/* 	} */
/* 	if (!scl) { */
/* 	  tval1 = 1.0 - (a[1]*a[1]); */
/* 	  tval2 = 4.0*a[2]; */
/* 	  tval3 = tval1/tval2; */
/* 	  tval4 = sqrt(tval3); */
/* 	  a[0] = temp*tval4; */
/* 	} */

}
