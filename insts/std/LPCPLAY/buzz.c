#include "ugens.h"

float buzz(float amp, float si, float hn, float *f, float *phs)
{
	int j,k;
	float q,d,h2n,h2np1;
	j = (int) *phs;
//	k = (j+1) % 1024;
	k = (j+1) & 1023;	// more efficient way for powers of 2
	h2n = 2.0 * hn; 
	h2np1 = h2n + 1.0; 
	q = (int)((*phs - (float)j) * h2np1)/h2np1;
	d = f[j];
	d += (f[k] - d) * q;
	if(!d)
		q = amp;
	else { 
		k = (long)(h2np1 * *phs) % 1024;
		q = amp * (f[k]/d - 1.)/h2n;
	} 
	*phs += si;
	while(*phs >= 1024.)  
		*phs -= 1024.;
	return(q);
}

float *
bbuzz(float amp, float si, float hn, float *f, float *phs, float *out, long alen)
{
	int i,j,k;
	float q,d;
	float one_over_h2n = 1.0f / (hn+hn);
	float h2np1 = (hn+hn) + 1.0f;
	float one_over_h2np1 = 1.0f / h2np1;
	float phase = *phs;
	float *fp = out;
	
	for(i=0; i<alen; i++) {
		j = (int) phase;
//		k = (j+1) % 1024;
		k = (j+1) & 1023;	// more efficient for powers of 2
		q = (int)((phase - (float)j) * h2np1) * one_over_h2np1;
//		d = f[j];
//		d += (f[k] - d) * q;
		d = f[j] + q * (f[k] - f[j]);
		if (d == 0.0f) {
			fp[i] = amp;
		}
		else {
//			k = (int)(h2np1 * phase) % 1024;
			k = (int)(h2np1 * phase) & 1023;
			fp[i] = amp * (f[k]/d - 1.0f) * one_over_h2n;
		}
		phase += si;
		while (phase >= 1024.0f)  
			phase -= 1024.0f;
	}
	*phs = phase;	// save phase
	return(out);
}
