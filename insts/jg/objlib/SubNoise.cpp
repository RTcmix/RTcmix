/* SubSampled Noise Generator Class, by Perry R. Cook, 1995-96
   White noise as often as you like.
   (Seed arg and method, and minor tweaks, added by JGG.)

   Returns pseudo-random numbers in the range (-1.0, 1.0).

   Note: This is like randh in other packages, but without amp scaling
   in tick and with freq specied in samples rather than Hz.  -JGG
*/

#include "SubNoise.h"


SubNoise :: SubNoise() : JGNoise(0)
{
   howOften = 15;
   counter = 0;       // was 15 in STK, so 1st ticks always gave 0 (by design?)
}


SubNoise :: ~SubNoise()
{
}


// (srate / freq in Hz to get subSample)

SubNoise :: SubNoise(int subSample, unsigned int aSeed = 0) : JGNoise(aSeed)
{    
   howOften = subSample;
   counter = 0;           // was == howOften in STK, so 1st ticks always gave 0
}


double SubNoise :: tick()
{
   if (!counter) {
      lastOutput = JGNoise::tick();
      counter = howOften - 1;                  // JGG: added -1
   }
   else
      counter--;

   return lastOutput;
}


// (srate / freq in Hz to get howOft)

void SubNoise :: setHowOften(int howOft)
{
   assert(howOft > 0);
   howOften = howOft;
}


