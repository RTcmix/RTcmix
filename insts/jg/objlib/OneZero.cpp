/* One Zero Filter Class, by Perry R. Cook, 1995-96
   The parameter <gain> is an additional gain parameter applied to the
   filter on top of the normalization that takes place automatically.
   So the net max gain through the system equals the value of <gain>.
   <sgain> is the combination of <gain> and the normalization parameter,
   so if you set the zeroCoeff to alpha, sgain is always set to
   gain / (1.0 + fabs(alpha)).
*/

#include "OneZero.h"


OneZero :: OneZero() : Filter(0)
{
   gain = 1.0;
   zeroCoeff = 1.0;
   sgain = 0.5;
   inputs = new double [1];
   this->clear();
}


OneZero :: ~OneZero()
{
   delete [] inputs;
}


void OneZero :: clear()
{
   inputs[0] = 0.0;
   lastOutput = 0.0;
}


void OneZero :: setGain(double aValue)
{
   gain = aValue;
   if (zeroCoeff > 0.0)                  // Normalize gain to 1.0 max
      sgain = gain / (1.0 + zeroCoeff);
   else
      sgain = gain / (1.0 - zeroCoeff);
}


void OneZero :: setCoeff(double aValue)
{
   zeroCoeff = aValue;
   if (zeroCoeff > 0.0)                  // Normalize gain to 1.0 max
      sgain = gain / (1.0 + zeroCoeff);
   else
      sgain = gain / (1.0 - zeroCoeff);
}


double OneZero :: tick(double sample)
{
   double temp = sgain * sample;
   lastOutput = (inputs[0] * zeroCoeff) + temp;
   inputs[0] = temp;
   return lastOutput;
}


