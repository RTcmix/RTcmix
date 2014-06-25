void
bbuzz(float amp, float si, float hn, double *f, float *phs, float *out, long alen)
{
	int i,j,k;
	double q,d;
	float one_over_h2n = 1.0f / (hn+hn);
	float h2np1 = (hn+hn) + 1.0f;
	float one_over_h2np1 = 1.0f / h2np1;
	float phase = *phs;
	float *fp = out;
	
	for(i=0; i<alen; i++) {
		j = (int) phase;
		k = (j+1) & 1023;	// more efficient for powers of 2
		q = (int)((phase - (float)j) * h2np1) * one_over_h2np1;
		d = f[j] + q * (f[k] - f[j]);
		if (d == 0.0) {
			fp[i] = amp;
		}
		else {
			k = (int)(h2np1 * phase) & 1023;
			fp[i] = amp * (f[k]/d - 1.0f) * one_over_h2n;
		}
		phase += si;
		while (phase >= 1024.0f)  
			phase -= 1024.0f;
	}
	*phs = phase;	// save phase
}
