int
boscili(float amp, float si, double *farray, int len, float *phs,
			float *array, int alen)      
{
	register int i,j,k;
	float frac,temp;
	float *fp = array; 

	temp = *phs;
	for (j=alen; j>0; --j) { 
		i = temp;        
		k = (i + 1) % len;  
		frac = temp  - i;      
		temp += si;                 
		while (temp >= len)
			temp -= len;       
 		*fp++ = ((*(farray+i) + (*(farray+k) - *(farray+i)) * frac) * amp);
	}
	*phs = temp;
	return (j);
}
