#include <math.h>

float hcomb(float xin,float rvt,float *a)
{
	float temp,hc;
	int l;
	if (rvt != a[2]) {
		if (rvt) a[3] = pow(.001,(a[1]/rvt));
		else  a[3] = 0.;
		a[2] = rvt;
	}
	if (a[4] >= a[0]) a[4] = 10.;
	else  a[4]++;
	l = a[4];
	hc = a[l];
	temp = a[3] * a[l];
	a[7] = a[8] * temp + a[6] - a[8] * a[7];
	a[l] = xin + a[7];                 
	a[6] = temp; 
	return(hc);    
}
