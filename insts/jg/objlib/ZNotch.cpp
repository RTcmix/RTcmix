/* An interpolating FIR comb filter class, by John Gibson, 1999.
   <scaler> scales the feed-forward term. It can be negative.
   The <delaySamps> argument to tick() gives the number of samples
   of delay for that call. This number can have a fractional part.
*/

#include "ZNotch.h"


ZNotch :: ZNotch(double srate, double loopTime, double scaler)
   : JGFilter(srate)
{
   long len;

   // make len extra long so caller can sweep way below loopTime
   len = (long)(loopTime * _sr * 2.0);

   _dline = new DLineL(len);
   _delsamps = -1.0;                    // force update on first tick()
   this->setScaler(scaler);

   this->clear();
}


ZNotch :: ~ZNotch()
{
   delete _dline;
}


void ZNotch :: clear()
{
   _dline->clear();
   lastOutput = 0.0;
}


void ZNotch :: setScaler(double scaler)
{
   _coef = scaler;
}


/* Make sure <delaySamps> is between 0 and (loopTime * srate * 2.0), or you'll
   get sudden pitch changes and dropouts.
*/
double ZNotch :: tick(double input, double delaySamps)
{
   if (delaySamps != _delsamps) {
      _dline->setDelay(delaySamps);
      _delsamps = delaySamps;
   }

   double temp = _dline->tick(input);
   lastOutput = input + (_coef * temp);

   return lastOutput;
}


