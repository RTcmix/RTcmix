#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "denoise.h"

/* -----------------------------------------------------------------
				cfsst.c

This is the FFT subroutine fsst.f from the IEEE library recoded in C.
	It takes a pointer to a vector b[i], for i = 0, 1, ..., N-1,
	and replaces it with its inverse DFT assuming real output.

-------------------------------------------------------------------*/

void
cfsst(float *b, long N)
{

/* This subroutine synthesizes the real vector b[k] for k=0, 1,
	..., N-1 from the fourier coefficients stored in the b
	array of size N+2.  The DC term is in location b[0] with
	b[1] equal to 0.  The i'th harmonic is a complex number
	stored as b[2*i] + j b[2*i+1].  The N/2 harmonic is in
	b[N] with b[N+1] equal to 0. The subroutine is called as
	fsst(b,N) where N=2**M and b is the real array described
	above.
*/
	extern double	pii, pi2, p7, p7two, c22, s22;

	float	pi8,
		invN;

	int	M;

	long	i,
		Nt,
		off;

	pii = 4. * atan(1.);
	pi2 = pii * 2.;
	pi8 = pii / 8.;
	p7 = 1. / sqrt(2.);
	p7two = 2. * p7;
	c22 = cos(pi8);
	s22 = sin(pi8);

/* determine M */

	for (i=1, Nt=2; i<20; i++,Nt *= 2) if (Nt==N) break;
	M = i;
	if (M==20){
		fprintf(stderr,"fast: N is not an allowable power of two\n");
		exit(1);
	}

	b[1] = b[N];

	for (i=3; i<N; i+=2) b[i] *= -1.;

/* scale the input by N */

	invN = 1. / N;
	for (i=0; i<N; i++) b[i] *= invN;

/* scramble the inputs */

	cford2(M,b);
	cford1(M,b);

/* perform radix 4 iterations */

	Nt = 4*N;
	if (M != 1) for (i=1; i<=M/2; i++){
		Nt /= 4;
		off = N / Nt;
		cfr4syn(off,Nt,b,b+off,b+2*off,b+3*off,b,b+off,b+2*off,b+3*off);
	}

/* do a radix 2 iteration if one is required */

	if (M%2 != 0){
		off = N / 2;
		cfr2tr(off,b,b+off);
	}
}

