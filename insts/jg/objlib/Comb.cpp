/* A non-interpolating IIR comb filter class, by John Gibson, 1999.
   <reverbTime> can be negative.
*/

#include "Comb.h"


Comb :: Comb(MY_FLOAT loopTime, MY_FLOAT reverbTime) : Filter()
{
   long len;

   loopt = loopTime;

   len = (long)(loopt * SR + 0.5);
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
   lastOutput = (MY_FLOAT) 0.0;
}


void Comb :: setReverbTime(MY_FLOAT reverbTime)
{
   if (reverbTime < 0.0)
      combCoeff = -((MY_FLOAT) pow(0.001, (double)(loopt / -reverbTime)));
   else
      combCoeff = (MY_FLOAT) pow(0.001, (double)(loopt / reverbTime));
}


MY_FLOAT Comb :: tick(MY_FLOAT input)
{
   MY_FLOAT temp;

   temp = input + (combCoeff * delayLine->lastOut());
   lastOutput = delayLine->tick(temp);

   return lastOutput;
}


