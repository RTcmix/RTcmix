/* Butterworth filter class, by John Gibson.
   Just a knock-off of clm and csound ones, which come right outta Dodge.
*/
#include "Butter.h"

#define SQRT2  1.4142135623730950488

enum {
   LOW_PASS,
   HIGH_PASS,
   BAND_PASS,
   BAND_REJECT
};


Butter :: Butter() : Filter()
{
   inputs = new MY_FLOAT [2];
   zeroCoeffs[0] = (MY_FLOAT) 0.0;
   zeroCoeffs[1] = (MY_FLOAT) 0.0;
   poleCoeffs[0] = (MY_FLOAT) 0.0;
   poleCoeffs[1] = (MY_FLOAT) 0.0;
   gain = (MY_FLOAT) 1.0;
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
   inputs[0] = (MY_FLOAT) 0.0;
   inputs[1] = (MY_FLOAT) 0.0;
   lastOutput = (MY_FLOAT) 0.0;
}


void Butter :: setLowPass(MY_FLOAT cutoff)
{
   c = 1.0 / tan((double)(cutoff * PI / SR));

   gain = (MY_FLOAT)(1.0 / (1.0 + SQRT_TWO * c + c * c));

   zeroCoeffs[0] = 2.0 * gain;
   zeroCoeffs[1] = gain;

   poleCoeffs[0] = 2.0 * (1.0 - c * c) * gain;
   poleCoeffs[1] = ((1.0 - SQRT_TWO * c) + c * c) * gain;

   type = LOW_PASS;
}


void Butter :: setHighPass(MY_FLOAT cutoff)
{
   c = tan((double)(cutoff * PI / SR));

   gain = (MY_FLOAT)(1.0 / (1.0 + SQRT_TWO * c + c * c));

   zeroCoeffs[0] = -2.0 * gain;
   zeroCoeffs[1] = gain;

   poleCoeffs[0] = 2.0 * (c * c - 1.0) * gain;
   poleCoeffs[1] = ((1.0 - SQRT_TWO * c) + c * c) * gain;

   type = HIGH_PASS;
}


void Butter :: setBandPassFreq(MY_FLOAT freq)
{
   assert(type == BAND_PASS);

   d = 2.0 * cos((double)(TWO_PI * freq / SR));

   poleCoeffs[0] = -c * d * gain;
}


void Butter :: setBandPassBandwidth(MY_FLOAT bandwidth)
{
   assert(type == BAND_PASS);

   c = 1.0 / tan((double)(bandwidth * PI / SR));

   gain = 1.0 / (1.0 + c);

   zeroCoeffs[0] = 0.0;
   zeroCoeffs[1] = -gain;

   poleCoeffs[0] = -c * d * gain;
   poleCoeffs[1] = (c - 1.0) * gain;
}


void Butter :: setBandPass(MY_FLOAT freq, MY_FLOAT bandwidth)
{
   d = 2.0 * cos((double)(TWO_PI * freq / SR));

   type = BAND_PASS;
   setBandPassBandwidth(bandwidth);
}


void Butter :: setBandRejectFreq(MY_FLOAT freq)
{
   assert(type == BAND_REJECT);

   d = 2.0 * cos((double)(TWO_PI * freq / SR));

   zeroCoeffs[0] = -d * gain;
   poleCoeffs[0] = zeroCoeffs[0];
}


void Butter :: setBandRejectBandwidth(MY_FLOAT bandwidth)
{
   assert(type == BAND_REJECT);

   c = tan((double)(bandwidth * PI / SR));

   gain = 1.0 / (1.0 + c);

   zeroCoeffs[0] = -d * gain;
   zeroCoeffs[1] = gain;

   poleCoeffs[0] = zeroCoeffs[0];
   poleCoeffs[1] = (1.0 - c) * gain;
}


void Butter :: setBandReject(MY_FLOAT freq, MY_FLOAT bandwidth)
{
   d = 2.0 * cos((double)(TWO_PI * freq / SR));

   type = BAND_REJECT;
   setBandRejectBandwidth(bandwidth);
}


//#define DISABLE_CLAMP_DENORMALS
#include "ClampDenormals.h"

MY_FLOAT Butter :: tick(MY_FLOAT sample)
{
   MY_FLOAT temp;

   temp = sample - inputs[0] * poleCoeffs[0] - inputs[1] * poleCoeffs[1];
   CLAMP_DENORMALS(temp);

   lastOutput = temp * gain + inputs[0] * zeroCoeffs[0]
                            + inputs[1] * zeroCoeffs[1];

   inputs[1] = inputs[0];
   inputs[0] = temp;

   return lastOutput;
}

