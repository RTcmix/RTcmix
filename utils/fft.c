#include <math.h>
#include "fft.h"
#include "../genlib/complexf.h"

int
fft(isi,nl,s) 
long isi;
long nl;
complex s[8192];
{
	complex w,t;
	long n,nv2;
	register long i,j,k,l,m;
	double pi,theta;
	n=(long)nl;
	pi=3.1415926535897932384;
	nv2=n/2;
	/* do bit reversal     */
	j=0;
	for (i=0;i<n;++i){
		if (i<j) {
			t=s[j];
			s[j]=s[i];
			s[i]=t;
		}
		k=nv2;
		while (j>=k){
			j-=k;
			k=(k+1)/2;
		}
		j+=k;
	}
	/* do butterfly */
	pi*=(double)isi;
	k=1;
	while(k<n){
		j=k*2;
		for(m=1;m<=k;++m){
			l=m-1;
			theta=(double)(pi*((double)l/(double)k));
			cmplx( cos(theta), sin(theta), w);
			for(i=l;i<n;i+=j){
				l=i+k;
				multiply( w, s[l], t);
				subtract( s[i], t, s[l]);
				add( s[i], t, s[i]);
			}
		} 
		k=j;
	}
	if(isi>0){
		theta=(double)n;
		theta=1./theta;
		pi=0.;
		cmplx( theta, 0., w);
		for(i=0;i<n;++i){
			multiply( w, s[i], s[i]);
		}
	}

	return 0;
}
