/* A non-interpolating FIR comb filter class, by John Gibson, 1999.
   <scaler> scales the feed-forward term. It can be negative. The effect
   is most pronounced when the scaler is 1.0 or -1.0, but is still much
   more subtle than that obtained with the IIR comb (i.e., Comb class).
*/

#include "Notch.h"


Notch :: Notch(double srate, MY_FLOAT loopTime, MY_FLOAT scaler) : Filter(srate)
{
   long len;

   len = (long) (loopTime * _sr + 0.5);
   delayLine = new DLineN(len);
   delayLine->setDelay(len);

   this->setScaler(scaler);

   this->clear();
}


Notch :: ~Notch()
{
   delete delayLine;
}


void Notch :: clear()
{
   delayLine->clear();
   lastOutput = (MY_FLOAT) 0.0;
}


void Notch :: setScaler(MY_FLOAT scaler)
{
   combCoeff = scaler;
}


MY_FLOAT Notch :: tick(MY_FLOAT input)
{
   MY_FLOAT temp;

   temp = delayLine->tick(input);
   lastOutput = input + (combCoeff * temp);

   return lastOutput;
}


