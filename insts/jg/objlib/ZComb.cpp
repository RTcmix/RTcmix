/* An interpolating IIR comb filter class, by John Gibson, 1999.
   The <delaySamps> argument to tick() gives the number of samples
   of delay for that call. This number can have a fractional part.
   <reverbTime> can be negative.
*/

#include "ZComb.h"


ZComb :: ZComb(MY_FLOAT loopTime, MY_FLOAT reverbTime) : Filter()
{
   long len;

   loopt = loopTime;

   // make len extra long so caller can sweep way below loopTime
   len = (long)(loopt * SR * 2.0);

   delayLine = new DLineL(len);
   delsamps = -1.0;                    // force update on first tick()
   this->setReverbTime(reverbTime);

   this->clear();
}


ZComb :: ~ZComb()
{
   delete delayLine;
}


void ZComb :: clear()
{
   delayLine->clear();
   lastOutput = (MY_FLOAT) 0.0;
}


void ZComb :: setReverbTime(MY_FLOAT reverbTime)
{
   if (reverbTime < 0.0)
      combCoeff = -((MY_FLOAT) pow(0.001, (double)(loopt / -reverbTime)));
   else
      combCoeff = (MY_FLOAT) pow(0.001, (double)(loopt / reverbTime));
}


/* Make sure <delaySamps> is between 0 and (loopTime * SR * 2.0), or you'll
   get sudden pitch changes and dropouts.
*/
MY_FLOAT ZComb :: tick(MY_FLOAT input, MY_FLOAT delaySamps)
{
   MY_FLOAT temp;

   if (delaySamps != delsamps) {
      delayLine->setDelay(delaySamps);
      delsamps = delaySamps;
   }

   temp = input + (combCoeff * delayLine->lastOut());
   lastOutput = delayLine->tick(temp);

   return lastOutput;
}


