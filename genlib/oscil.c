float oscil(float amp, float si, float *farray, int len, float *phs)
{
  int i = (int)*phs;
  *phs += si;
  while(*phs >= len)
	*phs -= len;     
  return(*(farray+i) * amp);
}
