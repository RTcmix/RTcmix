#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "denoise.h"

double	pii, pi2, p7, p7two, c22, s22;

/* -----------------------------------------------------------------
				cfast.c

This is the FFT subroutine fast.f from the IEEE library recoded in C.
	It takes a pointer to a real vector b[i], for i = 0, 1, ...,
	N-1, and replaces it with its finite discrete fourier transform.

-------------------------------------------------------------------*/

void
cfast(float *b, long N)
{

/* The DC term is returned in location b[0] with b[1] set to 0.
	Thereafter, the i'th harmonic is returned as a complex
	number stored as b[2*i] + j b[2*i+1].  The N/2 harmonic
	is returned in b[N] with b[N+1] set to 0.  Hence, b must
	be dimensioned to size N+2.  The subroutine is called as
	fast(b,N) where N=2**M and b is the real array described
	above.
*/

	float	pi8,
		tmp;

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

/* do a radix 2 iteration first if one is required */

	Nt = 1;
	if (M%2 != 0){
		Nt = 2;
		off = N / Nt;
		cfr2tr(off,b,b+off);
	}

/* perform radix 4 iterations */

	if (M != 1) for (i=1; i<=M/2; i++){
		Nt *= 4;
		off = N / Nt;
		cfr4tr(off,Nt,b,b+off,b+2*off,b+3*off,b,b+off,b+2*off,b+3*off);
	}

/* perform in-place reordering */

	cford1(M,b);
	cford2(M,b);

	tmp = b[1];
	b[1] = 0.;
	b[N] = tmp;
	b[N+1] = 0.;

	for (i=3; i<N; i+=2) b[i] *= -1.;
}

