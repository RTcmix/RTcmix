#include <math.h>
#include <combs.h>


void
combset(float SR, float loopt, float rvt,int init, float *a)
{
	int j;

	a[0] =  ((float)START + (loopt * SR + .5));
	a[1] = (float)pow(.001,(double)(loopt/rvt));
	if(!init) { 
		for(j=START; j<(int)*a; j++)  a[j] = 0;
		a[STARTM1] = START;
	}
}
