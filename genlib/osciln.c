float osciln(float amp, float si, float *farray, int len, float *phs)
{
	register i =  *phs;   
	*phs += si;            
	while(*phs >= len)
	       *phs -= len;     
	while(*phs < 0)
		*phs += len;
	return(*(farray+i) * amp); 

} 
