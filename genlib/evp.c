float evp(long nsample, double *f1, double *f2, float *q)
{
	register int jloc;
	float far = (float)(nsample) / *q;
	if (far > *(q+1)) {
		jloc = (1. - ((far - *(q+1))/ *(q+4))) * *(q+3);
		return (*(f2+jloc));
	}
	else
		if (far >= *(q+2)) return(1.);
	else {
		jloc = far * *(q+3)/ *(q+2);
		return (*(f1+jloc));
	}
}
