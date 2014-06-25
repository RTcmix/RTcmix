#include "pv.h"

/*
 * input I is a folded spectrum of length N; output O and
 * synthesis window W are of length Nw--overlap-add windowed,
 * unrotated, unfolded input data into output O
 * Since we know N is a power of 2, we can use a mask to wrap n.
 */
void overlapadd( float I[], int N, float W[], float O[], int Nw, int n )
{
	int i;
	register unsigned mask = (unsigned) N - 1;
    while ( n < 0 )
		n += N;
    n %= N;
    for (i = 0; i < Nw; ++i, ++n) {
		O[i] += I[n & mask] * W[i];
    }
}
