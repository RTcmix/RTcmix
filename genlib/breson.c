
float breson(float *x, float *a, float *out, int nvals)
{
	int i;
	for(i=0; i<nvals; i++) {
		out[i] = *a * x[i] + *(a+1) * *(a+3) - *(a+2) * *(a+4);
		*(a+4) = *(a+3);
		*(a+3) = out[i];
	}
	return(1.0); /* heck, I don't know... BGG */
}
