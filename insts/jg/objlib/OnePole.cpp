/* One Pole Filter Class, by Perry R. Cook, 1995-96
   (setFreq method added by JG)

   The parameter <gain> is an additional gain parameter applied to the
   filter on top of the normalization that takes place automatically.
   So the net max gain through the system equals the value of gain.
   <sgain> is the combination of gain and the normalization parameter,
   so if you set the poleCoeff to alpha, sgain is always set to
   gain * (1.0 - fabs(alpha)).

   (removed output[] -- function already served by lastOutput.  -JGG)
*/

#include "OnePole.h"


OnePole :: OnePole(double srate) : Filter(srate)
{
   poleCoeff = 0.9;
   gain = 1.0;
   sgain = 0.1;
   lastOutput = 0.0;
   outputs = inputs = NULL;                     // unused
}


OnePole :: ~OnePole()    
{
}


void OnePole :: clear()
{
   lastOutput = 0.0;
}


// positive freq for lowpass response, negative freq for highpass

void OnePole :: setFreq(double freq)
{
   // after Dodge, but with pole coeff sign flipped (for tick method)
   if (freq >= 0.0) {                       // lowpass
      double c = 2.0 - cos(freq * TWO_PI / _sr);
      poleCoeff = -(sqrt(c * c - 1.0) - c);
   }
   else {                                   // highpass
      double c = 2.0 + cos(freq * TWO_PI / _sr);
      poleCoeff = -(c - sqrt(c * c - 1.0));
   }

   if (poleCoeff > 0.0)                     // Normalize gain to 1.0 max
      sgain = gain * (1.0 - poleCoeff);
   else
      sgain = gain * (1.0 + poleCoeff);
}


void OnePole :: setPole(double aValue)
{
   poleCoeff = aValue;
   if (poleCoeff > 0.0)                     // Normalize gain to 1.0 max
      sgain = gain * (1.0 - poleCoeff);
   else
      sgain = gain * (1.0 + poleCoeff);
}


void OnePole :: setGain(double aValue)
{
   gain = aValue;
   if (poleCoeff > 0.0)                     // Normalize gain to 1.0 max
      sgain = gain * (1.0 - poleCoeff);
   else
      sgain = gain * (1.0 + poleCoeff);
}

#define DISABLE_CLAMP_DENORMALS  // until sure it works for doubles
#include "ClampDenormals.h"

double OnePole :: tick(double sample)
{
   lastOutput = (sgain * sample) + (poleCoeff * lastOutput);              
   CLAMP_DENORMALS(lastOutput);
   return lastOutput;
}


