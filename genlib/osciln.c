float osciln(float amp, float si, float *farray, int len, float *phs)
{
  int i = (int)*phs;
#ifdef DBUG
  printf("osciln(%2.2f,%2.2f,%x,%d,%2.2f\n",amp,si,farray,len,*phs);
#endif  
  *phs += si;
  while(*phs >= len)
	*phs -= len;     
  while(*phs < 0)
	*phs += len;
  return(*(farray+i) * amp); 
	
} 
