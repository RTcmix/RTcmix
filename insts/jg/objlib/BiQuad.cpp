/* BiQuad (2-pole, 2-zero) Filter Class, by Perry R. Cook, 1995-96
   See books on filters to understand more about how this works.
   Nothing out of the ordinary in this version.

   (Changed to use new/delete; added setFreqBandwidthAndGain and
   some comments.  -JGG)
*/

#include "BiQuad.h"


BiQuad :: BiQuad(double srate) : Filter(srate)
{
   inputs = new MY_FLOAT [2];
   zeroCoeffs[0] = (MY_FLOAT) 0.0;
   zeroCoeffs[1] = (MY_FLOAT) 0.0;
   poleCoeffs[0] = (MY_FLOAT) 0.0;
   poleCoeffs[1] = (MY_FLOAT) 0.0;
   gain = (MY_FLOAT) 1.0;
   this->clear();
}


BiQuad :: ~BiQuad()
{
   delete [] inputs;
}


void BiQuad :: clear()
{
   inputs[0] = (MY_FLOAT) 0.0;
   inputs[1] = (MY_FLOAT) 0.0;
   lastOutput = (MY_FLOAT) 0.0;
}


void BiQuad :: setPoleCoeffs(MY_FLOAT *coeffs)
{
   poleCoeffs[0] = coeffs[0];
   poleCoeffs[1] = coeffs[1];
}


void BiQuad :: setZeroCoeffs(MY_FLOAT *coeffs)
{
   zeroCoeffs[0] = coeffs[0];
   zeroCoeffs[1] = coeffs[1];
}


void BiQuad :: setEqualGainZeroes()
{
   zeroCoeffs[0] = (MY_FLOAT) 0.0;
   zeroCoeffs[1] = (MY_FLOAT) -1.0;
}


void BiQuad :: setGain(MY_FLOAT aValue)
{
   gain = aValue;
}


// Same as the TwoPole method
// <reson> is radius of pole on zplane; must be <= 1.0 for stable filter

void BiQuad :: setFreqAndReson(MY_FLOAT freq, MY_FLOAT reson)
{
   poleCoeffs[0] = (MY_FLOAT) (2.0 * reson * cos(TWO_PI * (double)(freq / _sr)));
   poleCoeffs[1] = -(reson * reson);
}


// (similar to make-formant in clm, but with bw in Hz instead of R)

void BiQuad :: setFreqBandwidthAndGain(MY_FLOAT freq, MY_FLOAT bw,
                                                               MY_FLOAT aGain)
{
   MY_FLOAT reson = (MY_FLOAT) (exp(-PI * (double)(bw / _sr)));

   setFreqAndReson(freq, reson);

   zeroCoeffs[0] = (MY_FLOAT) 0.0;
   zeroCoeffs[1] = (MY_FLOAT) -reson;

   // Note: using '+' rather than the '*' in 3/98 clm, which may be wrong (?)
   gain = aGain * ((MY_FLOAT) sin(TWO_PI * (double)(freq / _sr))
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

MY_FLOAT BiQuad :: tick(MY_FLOAT sample)
{
   MY_FLOAT temp;

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

