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


OnePole :: OnePole() : Filter()
{
   poleCoeff = (MY_FLOAT) 0.9;
   gain = (MY_FLOAT) 1.0;
   sgain = (MY_FLOAT) 0.1;
   lastOutput = (MY_FLOAT) 0.0;
   outputs = inputs = NULL;                     // unused
}


OnePole :: ~OnePole()    
{
}


void OnePole :: clear()
{
   lastOutput = (MY_FLOAT) 0.0;
}


// positive freq for lowpass response, negative freq for highpass

void OnePole :: setFreq(MY_FLOAT freq)
{
   // after Dodge, but with pole coeff sign flipped (for tick method)
   if (freq >= 0.0) {                       // lowpass
      double c = 2.0 - cos((double)(freq * TWO_PI / SR));
      poleCoeff = (MY_FLOAT) -(sqrt(c * c - 1.0) - c);
   }
   else {                                   // highpass
      double c = 2.0 + cos((double)(freq * TWO_PI / SR));
      poleCoeff = (MY_FLOAT) -(c - sqrt(c * c - 1.0));
   }

   if (poleCoeff > 0.0)                     // Normalize gain to 1.0 max
      sgain = gain * ((MY_FLOAT) 1.0 - poleCoeff);
   else
      sgain = gain * ((MY_FLOAT) 1.0 + poleCoeff);
}


void OnePole :: setPole(MY_FLOAT aValue)
{
   poleCoeff = aValue;
   if (poleCoeff > 0.0)                     // Normalize gain to 1.0 max
      sgain = gain * ((MY_FLOAT) 1.0 - poleCoeff);
   else
      sgain = gain * ((MY_FLOAT) 1.0 + poleCoeff);
}


void OnePole :: setGain(MY_FLOAT aValue)
{
   gain = aValue;
   if (poleCoeff > 0.0)                     // Normalize gain to 1.0 max
      sgain = gain * ((MY_FLOAT) 1.0 - poleCoeff);
   else
      sgain = gain * ((MY_FLOAT) 1.0 + poleCoeff);
}


MY_FLOAT OnePole :: tick(MY_FLOAT sample)
{
   lastOutput = (sgain * sample) + (poleCoeff * lastOutput);              
   return lastOutput;
}


