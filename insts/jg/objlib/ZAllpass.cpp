/* An interpolating allpass filter class, by John Gibson, 2003.
*/

#include "ZAllpass.h"


ZAllpass :: ZAllpass(MY_FLOAT loopTime, MY_FLOAT reverbTime) : Filter()
{
   long len;

   loopt = loopTime;

   // make len extra long so caller can sweep way below loopTime
   len = (long)(loopTime * SR * 2.0);

   delayLine = new DLineL(len);
   delsamps = -1.0;                    // force update on first tick()

   this->setReverbTime(reverbTime);

   this->clear();
}


ZAllpass :: ~ZAllpass()
{
   delete delayLine;
}


void ZAllpass :: clear()
{
   delayLine->clear();
   lastOutput = (MY_FLOAT) 0.0;
}


void ZAllpass :: setReverbTime(MY_FLOAT reverbTime)
{
   if (reverbTime < 0.0)
      allPassCoeff = -((MY_FLOAT) pow(0.001, (double)(loopt / -reverbTime)));
   else
      allPassCoeff = (MY_FLOAT) pow(0.001, (double)(loopt / reverbTime));
}


/* Make sure <delaySamps> is between 0 and (loopTime * SR * 2.0), or you'll
   get sudden pitch changes and dropouts.
*/
MY_FLOAT ZAllpass :: tick(MY_FLOAT input, MY_FLOAT delaySamps)
{
   MY_FLOAT temp;

   if (delaySamps != delsamps) {
      delayLine->setDelay(delaySamps);
      delsamps = delaySamps;
   }

   temp = delayLine->lastOut();
   lastOutput = input + (allPassCoeff * temp);
   delayLine->tick(lastOutput);
   lastOutput = temp - (allPassCoeff * lastOutput);

   return lastOutput;
}


