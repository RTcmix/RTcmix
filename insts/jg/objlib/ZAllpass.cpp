/* An interpolating allpass filter class, by John Gibson, 2003.
*/

#include "ZAllpass.h"


ZAllpass :: ZAllpass(double srate, double loopTime, double reverbTime)
   : JGFilter(srate)
{
   long len;

   loopt = loopTime;

   // make len extra long so caller can sweep way below loopTime
   len = (long) (loopTime * _sr * 2.0);

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
   lastOutput = 0.0;
}


void ZAllpass :: setReverbTime(double reverbTime)
{
   if (reverbTime < 0.0)
      allPassCoeff = -pow(0.001, loopt / -reverbTime);
   else
      allPassCoeff = pow(0.001, loopt / reverbTime);
}


/* Make sure <delaySamps> is between 0 and (loopTime * srate * 2.0), or you'll
   get sudden pitch changes and dropouts.
*/
double ZAllpass :: tick(double input, double delaySamps)
{
   if (delaySamps != delsamps) {
      delayLine->setDelay(delaySamps);
      delsamps = delaySamps;
   }

   double temp = delayLine->lastOut();
   lastOutput = input + (allPassCoeff * temp);
   delayLine->tick(lastOutput);
   lastOutput = temp - (allPassCoeff * lastOutput);

   return lastOutput;
}

