float *
ballpole(float *x, long *jcount, long npoles, float *past, float *c, float *out, long nvals)
{
	int i,nfint;
	long j;
	float *retval = out;
	float temp;
	for(i=0;i<nvals;++i){
		temp = *x++;
		for(j= *jcount, nfint=0;  nfint<npoles;  nfint++,j++)
			temp += (*(c+nfint) * *(past+j));
		*out++ =  *(past+ *jcount) = *(past+*jcount+npoles) = temp;
		*jcount = (*jcount + 1) % npoles;
		}
	return(retval);
}
