#include "../H/ugens.h"

float delget(float *a, float wait, int *l)
{
/*  get value from delay line, wait seconds old. */

	register int i = *l - (int)(wait * SR +.5);
	if(i < 0)  {
		i += *(l+1);
		if(i < 0) return(0);
		}
	return(*(a+i));
}
