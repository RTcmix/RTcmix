/* Two Pole Filter Class, by Perry R. Cook, 1995-96
   (setFreqBandwidthAndScale and comments added by JG)
*/
#include "TwoPole.h"


TwoPole :: TwoPole() : Filter()
{
   outputs = new MY_FLOAT [2];
   poleCoeffs[0] = (MY_FLOAT) 0.0;
   poleCoeffs[1] = (MY_FLOAT) 0.0;
   gain = (MY_FLOAT) 1.0;
   this->clear();
   inputs = NULL;           // unused
}


TwoPole :: ~TwoPole()
{
   delete [] outputs;
}


void TwoPole :: clear()
{
   outputs[0] = (MY_FLOAT) 0.0;
   outputs[1] = (MY_FLOAT) 0.0;
   lastOutput = (MY_FLOAT) 0.0;
}


// For stability: (-2.0 < coeffs[0] < 2.0) && (-1.0 < coeffs[1] < 1.0)

void TwoPole :: setPoleCoeffs(MY_FLOAT *coeffs)
{
   poleCoeffs[0] = coeffs[0];
   poleCoeffs[1] = coeffs[1];
}


void TwoPole :: setGain(MY_FLOAT aValue)
{
   gain = aValue;
}


// NOTE: same as CLM ppolar filter -- see clm.html
// freq is angle and reson (bandwidth) is radius of a pole
// 0 <= freq <= srate/2; 0 <= reson < 1
// To get (approx) reson from bandwidth (in hz): reson = exp(-PI * bw / SR);

void TwoPole :: setFreqAndReson(MY_FLOAT freq, MY_FLOAT reson)
{
   poleCoeffs[0] = (MY_FLOAT) (2.0 * reson * cos(TWO_PI * (double)(freq / SR)));
   poleCoeffs[1] = -(reson * reson);
}


// This is like the cmix genlib rsnset (and csound reson).

void TwoPole :: setFreqBandwidthAndScale(MY_FLOAT freq, MY_FLOAT bw, int scale)
{
   MY_FLOAT b1, b2, c;

   b2 = (MY_FLOAT) exp(-TWO_PI * bw / SR);
   c = 1.0 + b2;
   b1 = 4.0 * b2 / c * cos(TWO_PI * (double)freq / SR);

   if (scale == 1)            // for periodic signals
      gain = (1.0 - b2) * (MY_FLOAT) sqrt((double)(1.0 - b1 * b1 / 4.0 * b2));
   else if (scale == 2)       // for noise signals
      gain = (MY_FLOAT) sqrt((double)((1.0 - b2) / c * (c * c - b1 * b1)));
   else                       // leave unscaled
      gain = 1.0;

   poleCoeffs[0] = b1;
   poleCoeffs[1] = -b2;       // flip sign due to add in tick equation
}


// y0 = g x(n) + b1 y(n-1) + b2 y(n-2)
// Note: coeff signs flipped from usual

MY_FLOAT TwoPole :: tick(MY_FLOAT sample)
{
   MY_FLOAT temp;

   temp = sample * gain;
   temp += poleCoeffs[0] * outputs[0];
   temp += poleCoeffs[1] * outputs[1];
   outputs[1] = outputs[0];
   outputs[0] = temp;
   lastOutput = outputs[0];
   return lastOutput;
}


