/* Linear interpolating sub-sampled noise generator class, by John Gibson, 1999
   Gets new random numbers every <subSample> samples, and interpolates between
   the last and previous random numbers on each tick(). Returns numbers in the
   range (-1.0, 1.0).

   Note: This is like randi in other packages, but without amp scaling
   in tick and with freq specied in samples rather than Hz.  -JGG
*/

#include "SubNoiseL.h"


SubNoiseL :: SubNoiseL(int subSample, unsigned int aSeed)
           : SubNoise(subSample, aSeed)
{    
   assert(subSample > 0);

   prevRand = 0.0;
   curVal = 0.0;
   increment = 0.0;
}


SubNoiseL :: ~SubNoiseL()
{
}


double SubNoiseL :: tick()
{
   SubNoise::tick();                    // NB: lastOutput set here

   if (lastOutput != prevRand) {
      increment = (lastOutput - prevRand) / (double) howOften;
      prevRand = lastOutput;
   }
   curVal += increment;

   return curVal;
}


/* Need this here because we have to let SubNoise keep control of lastOutput */

double SubNoiseL :: lastOut()
{
   return curVal;
}


