#include "../H/ugens.h"

float dliget(float *a, float wait, int *l)
{
/* get interpolated value from delay line, wait seconds old */
	register int im1;
	float x = wait * SR;
	register int i = x;
	float frac = x - i;
	i = *l - i;
	im1 = i - 1;
	if(i <= 0) { 
		if(i < 0) i += *(l+1);
		if(i < 0) return(0.);
		if(im1 < 0) im1 += *(l+1);
		}
	return(*(a+i) + frac * (*(a+im1) - *(a+i)));
}
