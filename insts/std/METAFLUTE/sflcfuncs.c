#include <stdio.h>
#include <ugens.h>

void
mdelset(float *a, int *l, int imax)
{
/* delay initialization.  a is address of float array, l is size-2 int
 * array for bookkeeping variables, xmax, is maximum expected delay */

	int i;

	*l = 0;
	*(l+1) = imax;
	for(i = 0; i < *(l+1); i++) *(a+i) = 0;
}


float
mdelget(float *a, int samps, int *l)
{
/*  get value from delay line, samps samples delayed */

	register int i = *l - samps;

	if(i < 0)  {
		i += *(l+1);
		if(i < 0) return(0);
		}

	return(*(a+i));
}

float  
mdliget(float *a, float samps, int *l)
{
/* get interpolated value from delay line, wait seconds old */
	int im1;
	int i;
	float frac;

	i = samps;
	frac = samps - i;
	i = *l - i;
	im1 = i - 1;
	if(i <= 0) {
		if(i < 0) i += *(l+1);
		if(i < 0) return(0.);
		if(im1 < 0) im1 += *(l+1);
		}
	return(*(a+i) + frac * (*(a+im1) - *(a+i)));
}


