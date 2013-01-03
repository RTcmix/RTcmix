/* Two Pole Filter Class, by Perry R. Cook, 1995-96
   (setFreqBandwidthAndScale and comments added by JG)
*/
#include "TwoPole.h"


TwoPole :: TwoPole(double srate) : JGFilter(srate)
{
   outputs = new double [2];
   poleCoeffs[0] = 0.0;
   poleCoeffs[1] = 0.0;
   gain = 1.0;
   this->clear();
   inputs = NULL;           // unused
}


TwoPole :: ~TwoPole()
{
   delete [] outputs;
}


void TwoPole :: clear()
{
   outputs[0] = 0.0;
   outputs[1] = 0.0;
   lastOutput = 0.0;
}


// For stability: (-2.0 < coeffs[0] < 2.0) && (-1.0 < coeffs[1] < 1.0)

void TwoPole :: setPoleCoeffs(double *coeffs)
{
   poleCoeffs[0] = coeffs[0];
   poleCoeffs[1] = coeffs[1];
}


void TwoPole :: setGain(double aValue)
{
   gain = aValue;
}


// NOTE: same as CLM ppolar filter -- see clm.html
// freq is angle and reson (bandwidth) is radius of a pole
// 0 <= freq <= srate/2; 0 <= reson < 1
// To get (approx) reson from bandwidth (in hz): reson = exp(-PI * bw / srate);

void TwoPole :: setFreqAndReson(double freq, double reson)
{
   poleCoeffs[0] = 2.0 * reson * cos(TWO_PI * (freq / _sr));
   poleCoeffs[1] = -(reson * reson);
}


// This is like the cmix genlib rsnset (and csound reson).

void TwoPole :: setFreqBandwidthAndScale(double freq, double bw, int scale)
{
   double b2 = exp(-TWO_PI * bw / _sr);
   double c = 1.0 + b2;
   double b1 = 4.0 * b2 / c * cos(TWO_PI * freq / _sr);

   if (scale == 1)            // for periodic signals
      gain = (1.0 - b2) * sqrt(1.0 - b1 * b1 / 4.0 * b2);
   else if (scale == 2)       // for noise signals
      gain = sqrt((1.0 - b2) / c * (c * c - b1 * b1));
   else                       // leave unscaled
      gain = 1.0;

   poleCoeffs[0] = b1;
   poleCoeffs[1] = -b2;       // flip sign due to add in tick equation
}


// y0 = g x(n) + b1 y(n-1) + b2 y(n-2)
// Note: coeff signs flipped from usual

double TwoPole :: tick(double sample)
{
   double temp = sample * gain;
   temp += poleCoeffs[0] * outputs[0];
   temp += poleCoeffs[1] * outputs[1];
   outputs[1] = outputs[0];
   outputs[0] = temp;
   lastOutput = outputs[0];
   return lastOutput;
}


