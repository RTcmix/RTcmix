float *
bbuzz(float amp, float si, float hn, float *f, float *phs, float *a, long alen)
{
	register i,j,k;
	float q,d,h2n,h2np1;
	float *fp= a;
	h2n = hn+hn;
	h2np1 = h2n+1.;
	for(i=0; i<alen; i++) {
		j = *phs;
		k = (j+1) % 1024;
		q = (int)((*phs - (float)j) * h2np1)/h2np1;
		d = *(f+j);
		d += (*(f+k)-d)*q;
		if(!d) *fp++  = amp;
		else { 
			k = (long)(h2np1 * *phs) % 1024;
			*fp++  = amp * (*(f+k)/d - 1.)/h2n;
		} 
		*phs += si;
		while(*phs >= 1024.)  
			*phs -= 1024.;
	}
	return(a);
}
