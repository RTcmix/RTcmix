/* An interpolating IIR comb filter class, by John Gibson, 1999.
   The <delaySamps> argument to tick() gives the number of samples
   of delay for that call. This number can have a fractional part.
   <reverbTime> can be negative.
*/

#include "ZComb.h"


ZComb :: ZComb(double srate, double loopTime, double reverbTime)
   : JGFilter(srate)
{
   long len;

   loopt = loopTime;

   // make len extra long so caller can sweep way below loopTime
   len = (long) (loopt * _sr * 2.0);

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
   lastOutput = 0.0;
}


void ZComb :: setReverbTime(double reverbTime)
{
   if (reverbTime < 0.0)
      combCoeff = -pow(0.001, loopt / -reverbTime);
   else
      combCoeff = pow(0.001, loopt / reverbTime);
}


/* Make sure <delaySamps> is between 0 and (loopTime * srate * 2.0), or you'll
   get sudden pitch changes and dropouts.
*/
double ZComb :: tick(double input, double delaySamps)
{
   if (delaySamps != delsamps) {
      delayLine->setDelay(delaySamps);
      delsamps = delaySamps;
   }

   double temp = input + (combCoeff * delayLine->lastOut());
   lastOutput = delayLine->tick(temp);

   return lastOutput;
}

