/* A non-interpolating allpass filter class, by John Gibson, 1999.
*/

#include "Allpass.h"


Allpass :: Allpass(MY_FLOAT loopTime, MY_FLOAT reverbTime) : Filter()
{
   long len;

   loopt = loopTime;

   len = (long)(loopt * SR + 0.5);
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
   lastOutput = (MY_FLOAT) 0.0;
}


void Allpass :: setReverbTime(MY_FLOAT reverbTime)
{
   if (reverbTime < 0.0)
      allPassCoeff = -((MY_FLOAT) pow(0.001, (double)(loopt / -reverbTime)));
   else
      allPassCoeff = (MY_FLOAT) pow(0.001, (double)(loopt / reverbTime));
}


MY_FLOAT Allpass :: tick(MY_FLOAT input)
{
   MY_FLOAT temp;

   temp = delayLine->lastOut();
   lastOutput = input + (allPassCoeff * temp);
   delayLine->tick(lastOutput);
   lastOutput = temp - (allPassCoeff * lastOutput);

   return lastOutput;
}


