float osciln(float amp, float si, double *farray, int len, float *phs)
{
	register int i =  *phs;   
	*phs += si;            
	while(*phs >= len)
	       *phs -= len;     
	while(*phs < 0)
		*phs += len;
	return(*(farray+i) * amp); 

} 
