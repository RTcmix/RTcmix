/* Linear interpolating sub-sampled noise generator class, by John Gibson, 1999
   Gets new random numbers every <subSample> samples, and interpolates between
   the last and previous random numbers on each tick(). Returns numbers in the
   range (-1.0, 1.0).

   Note: This is like randi in other packages, but without amp scaling
   in tick and with freq specied in samples rather than Hz.  -JGG
*/

#include "SubNoiseL.h"


SubNoiseL :: SubNoiseL(int subSample = 15, unsigned int aSeed = 0)
           : SubNoise(subSample, aSeed)
{    
   assert(subSample > 0);

   prevRand = (MY_FLOAT) 0.0;
   curVal = (MY_FLOAT) 0.0;
   increment = (MY_FLOAT) 0.0;
}


SubNoiseL :: ~SubNoiseL()
{
}


MY_FLOAT SubNoiseL :: tick()
{
   SubNoise::tick();                    // NB: lastOutput set here

   if (lastOutput != prevRand) {
      increment = (lastOutput - prevRand) / (MY_FLOAT) howOften;
      prevRand = lastOutput;
   }
   curVal += increment;

   return curVal;
}


/* Need this here because we have to let SubNoise keep control of lastOutput */

MY_FLOAT SubNoiseL :: lastOut()
{
   return curVal;
}


