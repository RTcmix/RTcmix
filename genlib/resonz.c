#include <math.h>

void
rszset(float SR, float cf, float bw, float xinit, float *a)
{
      double exp(),cos();
      int i;
      a[0]=exp(-M_PI*bw/SR);
      a[1]=1.-a[0];
      a[2]=2.*a[0]*cos(2.*M_PI*cf/SR);
      a[3] = -a[0]*a[0];
      if(!xinit) for(i=4; i<9; i++) a[i]=0.;
}

float resonz(float sig, float *a)
{
      float z;
      z=a[1]*(sig-a[0]*a[4])+a[2]*a[5]+a[3]*a[6];
      a[4]=a[8];
      a[8]=sig;
      a[6]=a[5];
      a[5]=z;
      return(z);
}

void bresonz(float *sig, float *a, float *out, int count)
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
