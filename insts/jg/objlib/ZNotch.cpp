/* An interpolating FIR comb filter class, by John Gibson, 1999.
   <scaler> scales the feed-forward term. It can be negative.
   The <delaySamps> argument to tick() gives the number of samples
   of delay for that call. This number can have a fractional part.
*/

#include "ZNotch.h"


ZNotch :: ZNotch(MY_FLOAT loopTime, MY_FLOAT scaler) : Filter()
{
   long len;

   // make len extra long so caller can sweep way below loopTime
   len = (long)(loopTime * SR * 2.0);

   delayLine = new DLineL(len);
   delsamps = -1.0;                    // force update on first tick()
   this->setScaler(scaler);

   this->clear();
}


ZNotch :: ~ZNotch()
{
   delete delayLine;
}


void ZNotch :: clear()
{
   delayLine->clear();
   lastOutput = (MY_FLOAT) 0.0;
}


void ZNotch :: setScaler(MY_FLOAT scaler)
{
   combCoeff = scaler;
}


/* Make sure <delaySamps> is between 0 and (loopTime * SR * 2.0), or you'll
   get sudden pitch changes and dropouts.
*/
MY_FLOAT ZNotch :: tick(MY_FLOAT input, MY_FLOAT delaySamps)
{
   MY_FLOAT temp;

   if (delaySamps != delsamps) {
      delayLine->setDelay(delaySamps);
      delsamps = delaySamps;
   }

   temp = delayLine->tick(input);
   lastOutput = input + (combCoeff * temp);

   return lastOutput;
}


