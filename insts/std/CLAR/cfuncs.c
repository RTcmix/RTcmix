#include <stdio.h>
#include <ugens.h>

void
mdelset(float SR, float *a, int *l, int imax)
{
/* delay initialization.  a is address of float array, l is size-2 int
 * array for bookkeeping variables, imax, is maximum expected delay in samps */

	int i;

	l[0] = 0;
	l[1] = SR;
	l[2] = imax;
	for(i = 0; i < l[2]; i++) a[i] = 0;
}


float
mdelget(float *a, int samps, int *l)
{
/*  get value from delay line, samps samples delayed */

	register int i = l[0] - samps;

	if(i < 0)  {
		i += l[2];
		if(i < 0) return(0);
		}

	return(a[i]);
}

float  
mdliget(float *a, float samps, int *l)
{
/* get interpolated value from delay line, samps samples old */
	int im1;
	int i;
	float frac;

	i = samps;
	frac = samps - i;
	i = l[0] - i;
	im1 = i - 1;
	if(i <= 0) {
		if(i < 0) i += l[2];
		if(i < 0) return(0.);
		if(im1 < 0) im1 += l[2];
		}
	return(a[i] + frac * (a[im1] - a[i]));
}

