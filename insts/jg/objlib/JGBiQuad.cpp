/* JGBiQuad (2-pole, 2-zero) Filter Class, by Perry R. Cook, 1995-96
   See books on filters to understand more about how this works.
   Nothing out of the ordinary in this version.

   (Changed to use new/delete; added setFreqBandwidthAndGain and
   some comments.  -JGG)
*/

#include "JGBiQuad.h"


JGBiQuad :: JGBiQuad(double srate) : JGFilter(srate)
{
   inputs = new double [2];
   zeroCoeffs[0] = 0.0;
   zeroCoeffs[1] = 0.0;
   poleCoeffs[0] = 0.0;
   poleCoeffs[1] = 0.0;
   gain = 1.0;
   this->clear();
}


JGBiQuad :: ~JGBiQuad()
{
   delete [] inputs;
}


void JGBiQuad :: clear()
{
   inputs[0] = 0.0;
   inputs[1] = 0.0;
   lastOutput = 0.0;
}


void JGBiQuad :: setPoleCoeffs(double *coeffs)
{
   poleCoeffs[0] = coeffs[0];
   poleCoeffs[1] = coeffs[1];
}


void JGBiQuad :: setZeroCoeffs(double *coeffs)
{
   zeroCoeffs[0] = coeffs[0];
   zeroCoeffs[1] = coeffs[1];
}


void JGBiQuad :: setEqualGainZeroes()
{
   zeroCoeffs[0] = 0.0;
   zeroCoeffs[1] = -1.0;
}


void JGBiQuad :: setGain(double aValue)
{
   gain = aValue;
}


// Same as the TwoPole method
// <reson> is radius of pole on zplane; must be <= 1.0 for stable filter

void JGBiQuad :: setFreqAndReson(double freq, double reson)
{
   poleCoeffs[0] = 2.0 * reson * cos(TWO_PI * (freq / _sr));
   poleCoeffs[1] = -(reson * reson);
}


// (similar to make-formant in clm, but with bw in Hz instead of R)

void JGBiQuad :: setFreqBandwidthAndGain(double freq, double bw, double aGain)
{
   double reson = exp(-PI * (bw / _sr));

   setFreqAndReson(freq, reson);

   zeroCoeffs[0] = 0.0;
   zeroCoeffs[1] = -reson;

   // Note: using '+' rather than the '*' in 3/98 clm, which may be wrong (?)
   gain = aGain * (sin(TWO_PI * (freq / _sr))
#if 1 // see clm mus.lisp ... they use the 2nd one
                   + (1.0 - reson));
#else
                   + (1.0 - (reson * reson)));
#endif

/* cmix rszset has:
   gain = 1.0 - reson;
*/
}


// y0 = g x(n) + a1 x(n-1) + a2 x(n-2) + b1 y(n-1) + b2 y(n-2)
// Note: signs of b1 and b2 flipped compared to what you often see
// Note: This is the canonical form, needing only 2 history vals.

double JGBiQuad :: tick(double sample)
{
   double temp;

   temp = sample * gain;
   temp += inputs[0] * poleCoeffs[0];
   temp += inputs[1] * poleCoeffs[1];

   lastOutput = temp;
   lastOutput += (inputs[0] * zeroCoeffs[0]);
   lastOutput += (inputs[1] * zeroCoeffs[1]);

   inputs[1] = inputs[0];
   inputs[0] = temp;

   return lastOutput;
}

