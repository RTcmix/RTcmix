#include "../../H/ugens.h"


float bresonz(sig,a,out,count)
float *sig,*a,*out;
{
	float z;
	int i;
	for(i=0; i<count; i++) {
		z=a[1]*(sig[i]-a[0]*a[4])+a[2]*a[5]+a[3]*a[6];
		a[4]=a[8];
		a[8]=sig[i];
		a[6]=a[5];
		out[i] = a[5] = z;
	}
}
