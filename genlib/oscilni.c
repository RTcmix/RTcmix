float oscilni(float amp, float si, float *farray, int len, float *phs)
{
	register i =  *phs;         
        register k =  (i + 1) % len; 
	float frac = *phs  - i;       
	*phs += si;         
	while(*phs >= len)
		*phs -= len;  
	while(*phs < 0)
		*phs += len;
	return((*(farray+i) + (*(farray+k) - *(farray+i)) *
					   frac) * amp);

}
