
float allpole(float x, int *jcount, int npoles, float *past, float *c)
{
	register int j,nfint;
	for(j= *jcount, nfint=0;  nfint<npoles;  nfint++,j++)
		x += (*(c+nfint) * *(past+j));
	*(past+ *jcount) = *(past+*jcount+npoles) = x;
	*jcount = (*jcount + 1) % npoles;
	return(x);
}
