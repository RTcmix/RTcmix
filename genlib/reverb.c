#include <macros.h>

float reverb(float xin, float *a)
{
	float *apoint,w,x,y,z;

	apoint = a;
	COMB(w,xin,apoint);
	apoint += (int)*apoint;
	COMB(x,xin,apoint);
	apoint += (int)*apoint;
	COMB(y,xin,apoint);
	apoint += (int)*apoint;
	COMB(z,xin,apoint);
	apoint += (int)*apoint;
	w += x+y+z;
	ALLPASS(x,w,apoint);
	apoint += (int)*apoint;
	ALLPASS(y,x*.25,apoint);
	return(y);
}
