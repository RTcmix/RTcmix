#include <math.h>

void rsnset(float SR, float cf, float bw, float scl, float xinit, float a[])
{
	float c, temp;

	if(!xinit) {
		a[4] = 0;
		a[3] = 0;
		}

	a[2] = (float)exp(-2.0f * M_PI * bw/SR);
	temp = 1. - a[2];
	c = a[2] + 1;
	a[1] = 4. * a[2]/c * (float)cos(2.0f * M_PI * cf/SR);
	if(scl < 0) a[0] = 1;
	if(scl) a[0] = sqrt(temp/c*(c*c-a[1]*a[1]));
	if(!scl) a[0] = temp*sqrt(1.-a[1]*a[1]/(4.*a[2]));
}
