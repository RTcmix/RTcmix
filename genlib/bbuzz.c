/* FIXME: This function requires that the <f> array contain exactly 1024
   elements. It means a score creating this array with a makegen would
   have to size it as 1024. (E.g., IIR's BUZZ inst.) Should add another
   arg that gives size of array.   -JGG
*/
float *
bbuzz(float amp, float si, float hn, double *f, float *phs, float *a, long alen)
{
	register int i,j,k;
	float q,d,h2n,h2np1;
	float *fp = a;

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
