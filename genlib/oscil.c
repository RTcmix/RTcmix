float oscil(float amp, float si, double *farray, int len, float *phs)
{
	register int i =  *phs;   
	*phs += si;            
	while(*phs >= len)
	       *phs -= len;     
	return(*(farray+i) * amp);
}
