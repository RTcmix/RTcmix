/* A non-interpolating IIR comb filter class, by John Gibson, 1999.
   <reverbTime> can be negative.
*/

#include "Comb.h"


Comb :: Comb(double srate, double loopTime, double reverbTime)
   : JGFilter(srate)
{
   long len;

   loopt = loopTime;

   len = (long) (loopt * _sr + 0.5);
   delayLine = new DLineN(len);
   delayLine->setDelay(len);

   this->setReverbTime(reverbTime);

   this->clear();
}


Comb :: ~Comb()
{
   delete delayLine;
}


void Comb :: clear()
{
   delayLine->clear();
   lastOutput = 0.0;
}


void Comb :: setReverbTime(double reverbTime)
{
   if (reverbTime < 0.0)
      combCoeff = -pow(0.001, loopt / -reverbTime);
   else
      combCoeff = pow(0.001, loopt / reverbTime);
}


double Comb :: tick(double input)
{
   double temp;

   temp = input + (combCoeff * delayLine->lastOut());
   lastOutput = delayLine->tick(temp);

   return lastOutput;
}


