/* A non-interpolating allpass filter class, by John Gibson, 1999.
*/

#include "Allpass.h"


Allpass :: Allpass(double srate, double loopTime, double reverbTime)
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


Allpass :: ~Allpass()
{
   delete delayLine;
}


void Allpass :: clear()
{
   delayLine->clear();
   lastOutput = 0.0;
}


void Allpass :: setReverbTime(double reverbTime)
{
   if (reverbTime < 0.0)
      allPassCoeff = -pow(0.001, loopt / -reverbTime);
   else
      allPassCoeff = pow(0.001, loopt / reverbTime);
}


double Allpass :: tick(double input)
{
   double temp;

   temp = delayLine->lastOut();
   lastOutput = input + (allPassCoeff * temp);
   delayLine->tick(lastOutput);
   lastOutput = temp - (allPassCoeff * lastOutput);

   return lastOutput;
}


