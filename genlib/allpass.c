
#include <combs.h>

float allpass(float samp,float *a)
{
	float temp,*aptr;
	if ( a[STARTM1] >= (int) a[0]) a[STARTM1] = START;
	aptr = a + (int)a[STARTM1];
	a[STARTM1] ++; 
	temp = *aptr;
	*aptr = *aptr * a[1] + samp;
	return(temp - a[1] * *aptr);
}
