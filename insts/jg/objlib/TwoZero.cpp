/*******************************************/
/*  Two Zero Filter Class,                 */
/*  by Perry R. Cook, 1995-96              */ 
/*  See books on filters to understand     */
/*  more about how this works.  Nothing    */
/*  out of the ordinary in this version.   */
/*******************************************/

#include "TwoZero.h"


TwoZero :: TwoZero(double srate) : JGFilter(srate)
{
   inputs = new double [2];
   zeroCoeffs[0] = 0.0;
   zeroCoeffs[1] = 0.0;
   gain = 1.0;
   this->clear();
   outputs = NULL;             // unused
}


TwoZero :: ~TwoZero()
{
   delete [] inputs;
}


void TwoZero :: clear()
{
   inputs[0] = 0.0;
   inputs[1] = 0.0;
   lastOutput = 0.0;
}


void TwoZero :: setZeroCoeffs(double *coeffs)
{
   zeroCoeffs[0] = coeffs[0];
   zeroCoeffs[1] = coeffs[1];
}


void TwoZero :: setGain(double aValue)
{
   gain = aValue;
}


// see csound areson (ugens5.c) for the friendlier version
// NO! arith in y0 = ... equation too different from tick below?


// NOTE: same as CLM zpolar filter -- see clm.html
// freq is angle and width (notchwidth) is radius of a zero
// 0 <= freq <= srate/2; 0 <= width < 1
// To get (approx) width from bandwidth (in hz): width = exp(-PI * bw / srate);

void TwoZero :: setFreqAndWidth(double freq, double width)
{
   zeroCoeffs[0] = -2.0 * width * cos(TWO_PI * (freq / _sr));
   zeroCoeffs[1] = width * width;
}


// y0 = g x(n) + a1 x(n-1) + a2 x(n-2)
// Note: history takes g x(n), not x(n)

double TwoZero :: tick(double sample)
{
   lastOutput = zeroCoeffs[0] * inputs[0];
   lastOutput += zeroCoeffs[1] * inputs[1];
   inputs[1] = inputs[0];
   inputs[0] = gain * sample;
   lastOutput += inputs[0];
   return lastOutput;
}


