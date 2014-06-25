#include "pv.h"
/*
 * evaluate magnitude of transfer function at frequency omega
 */
float lpamp( float omega, float a0, float *coef, int M )
{
 register float wpr, wpi, wr, wi, re, im, temp ;
 register int i ;
    if ( a0 == 0. )
	return( 0. ) ;
    wpr = cos( omega ) ;
    wpi = sin( omega ) ;
    re = wr = 1. ;
    im = wi = 0. ;
    for ( coef++, i = 1 ; i <= M ; i++, coef++ ) {
	wr = (temp = wr)*wpr - wi*wpi ;
	wi = wi*wpr + temp*wpi ;
	re += *coef*wr ;
	im += *coef*wi ;
    }
    if ( re == 0. && im == 0. )
	return( 0. ) ;
    else
	return( sqrt( a0/(re*re + im*im) ) ) ;
}
