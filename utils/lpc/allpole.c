float allpole(x,jcount,npoles,past,c)
float x,*past,*c;
long  *jcount,npoles;
{
	int j,nfint;
	for(j= *jcount, nfint=0;  nfint<npoles;  nfint++,j++)
		x += (*(c+nfint) * *(past+j));
	*(past+ *jcount) = *(past+*jcount+npoles) = x;
	*jcount = (*jcount + 1) % npoles;
	return(x);
}
float ballpole(x,jcount,npoles,past,c,out,nvals)
float *x,*past,*c,*out;
long  *jcount,npoles,nvals;
{
	register i,j,nfint;
	register float temp;
	for(i=0;i<nvals;++i){
		temp = *x++;
		for(j= *jcount, nfint=0;  nfint<npoles;  nfint++,j++)
			temp += (*(c+nfint) * *(past+j));
		*out++ =  *(past+ *jcount) = *(past+*jcount+npoles) = temp;
		*jcount = (*jcount + 1) % npoles;
		}
}
