/* Butterworth filter class, by John Gibson.
   Just a knock-off of clm and csound ones, which come right outta Dodge.
*/
#include "Butter.h"
#include <assert.h>

#define SQRT2  1.4142135623730950488

enum {
   LOW_PASS,
   HIGH_PASS,
   BAND_PASS,
   BAND_REJECT
};


Butter :: Butter(double srate) : JGFilter(srate)
{
   inputs = new double [2];
   zeroCoeffs[0] = 0.0;
   zeroCoeffs[1] = 0.0;
   poleCoeffs[0] = 0.0;
   poleCoeffs[1] = 0.0;
   gain = 1.0;
   type = LOW_PASS;
   c = d = 0.0;
   this->clear();
}


Butter :: ~Butter()
{
   delete [] inputs;
}


void Butter :: clear()
{
   inputs[0] = 0.0;
   inputs[1] = 0.0;
   lastOutput = 0.0;
}


void Butter :: setLowPass(double cutoff)
{
   if (cutoff <= 0.0)
      cutoff = 0.001;

   c = 1.0 / tan(cutoff * PI / _sr);

   gain = 1.0 / (1.0 + SQRT_TWO * c + c * c);

   zeroCoeffs[0] = 2.0 * gain;
   zeroCoeffs[1] = gain;

   poleCoeffs[0] = 2.0 * (1.0 - c * c) * gain;
   poleCoeffs[1] = ((1.0 - SQRT_TWO * c) + c * c) * gain;

   type = LOW_PASS;
}


void Butter :: setHighPass(double cutoff)
{
   c = tan(cutoff * PI / _sr);

   gain = 1.0 / (1.0 + SQRT_TWO * c + c * c);

   zeroCoeffs[0] = -2.0 * gain;
   zeroCoeffs[1] = gain;

   poleCoeffs[0] = 2.0 * (c * c - 1.0) * gain;
   poleCoeffs[1] = ((1.0 - SQRT_TWO * c) + c * c) * gain;

   type = HIGH_PASS;
}


void Butter :: setBandPassFreq(double freq)
{
   assert(type == BAND_PASS);

   d = 2.0 * cos(TWO_PI * freq / _sr);

   poleCoeffs[0] = -c * d * gain;
}


void Butter :: setBandPassBandwidth(double bandwidth)
{
   assert(type == BAND_PASS);

   c = 1.0 / tan(bandwidth * PI / _sr);

   gain = 1.0 / (1.0 + c);

   zeroCoeffs[0] = 0.0;
   zeroCoeffs[1] = -gain;

   poleCoeffs[0] = -c * d * gain;
   poleCoeffs[1] = (c - 1.0) * gain;
}


void Butter :: setBandPass(double freq, double bandwidth)
{
   d = 2.0 * cos(TWO_PI * freq / _sr);

   type = BAND_PASS;
   setBandPassBandwidth(bandwidth);
}


void Butter :: setBandRejectFreq(double freq)
{
   assert(type == BAND_REJECT);

   d = 2.0 * cos(TWO_PI * freq / _sr);

   zeroCoeffs[0] = -d * gain;
   poleCoeffs[0] = zeroCoeffs[0];
}


void Butter :: setBandRejectBandwidth(double bandwidth)
{
   assert(type == BAND_REJECT);

   c = tan(bandwidth * PI / _sr);

   gain = 1.0 / (1.0 + c);

   zeroCoeffs[0] = -d * gain;
   zeroCoeffs[1] = gain;

   poleCoeffs[0] = zeroCoeffs[0];
   poleCoeffs[1] = (1.0 - c) * gain;
}


void Butter :: setBandReject(double freq, double bandwidth)
{
   d = 2.0 * cos(TWO_PI * freq / _sr);

   type = BAND_REJECT;
   setBandRejectBandwidth(bandwidth);
}


#define DISABLE_CLAMP_DENORMALS  // disabled until sure it works with doubles
#include "ClampDenormals.h"

double Butter :: tick(double sample)
{
   double temp;

   temp = sample - inputs[0] * poleCoeffs[0] - inputs[1] * poleCoeffs[1];
   CLAMP_DENORMALS(temp);

   lastOutput = temp * gain + inputs[0] * zeroCoeffs[0]
                            + inputs[1] * zeroCoeffs[1];

   inputs[1] = inputs[0];
   inputs[0] = temp;

   return lastOutput;
}

