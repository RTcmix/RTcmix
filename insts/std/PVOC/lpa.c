#include "pv.h"

/*
 * given N samples of digital waveform x, lpa computes M+1 coefficients
 * by maximum entropy (autocorrelation) method for spectral estimation--
 * these are returned in b[] (b[0] is always set to 1). lpa itself returns
 * the a0 (residual energy) coefficient.
 */
float lpa( float x[], int N, float b[], int M )
{
 int i, j ;
 float s, at ;
 float a0, *rx, *rc ;

    rx = (float *) malloc( (M+2)*sizeof(float) ) ;
    rc = (float *) malloc( (M+2)*sizeof(float) ) ;
    for ( i = 0 ; i <= M + 1 ; i++ )
	for ( rx[i] = j = 0 ; j < N - i ; j++ )
	    rx[i] += x[j]*x[i + j] ;
    b[0] = 1. ;
    b[1] = rc[0] = rx[0] != 0. ? -rx[1]/rx[0] : 0. ;
    a0 = rx[0] + rx[1]*rc[0] ;
    for ( i = 1 ; i < M ; i++ ) {
	for ( s = j = 0 ; j <= i ; j++ )
	    s += rx[i - j + 1]*b[j] ;
	rc[i] = a0 != 0. ? -s/a0 : 0. ;
	for ( j = 1 ; j <= (i + 1)>>1 ; j++ ) {
	    at = b[j] + rc[i]*b[i-j+1] ;
	    b[i-j+1] += rc[i]*b[j] ;
	    b[j] = at ;
	}
	b[i+1] = rc[i] ;
	a0 += rc[i]*s ;
/*
	if ( a0 <= 0. ) {
	    fprintf( stderr, "lpa: matrix singularity" ) ;
	    return( a0 ) ;
	}
*/
    }
    free( rx ) ;
    free( rc ) ;
    return( a0 ) ;
}
