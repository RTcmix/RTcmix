/* RMS power gauge, based on the one in csound.  -JGG */

#include "RMS.h"


RMS :: RMS(double srate) : Filter(srate)
{
   gain = 1.0;
   subLowFilter = new OnePole(srate);
   subLowFilter->setFreq(10.0);
   windowSize = DEFAULT_CONTROL_RATE;
   counter = 0;
   lastOutput = 0.0;
   outputs = inputs = NULL;                    // unused
}


RMS :: ~RMS()    
{
   delete subLowFilter;
}


void RMS :: clear()
{
   lastOutput = 0.0;
   counter = 0;
}


void RMS :: setFreq(MY_FLOAT freq)
{
   subLowFilter->setFreq(freq);
}


void RMS :: setWindowSize(int nsamples)
{
   windowSize = nsamples;
   counter = 0;
}


MY_FLOAT RMS :: tick(MY_FLOAT sample)
{
   MY_FLOAT temp;

   temp = subLowFilter->tick(sample * sample);
   if (--counter < 0) {
      lastOutput = (MY_FLOAT) sqrt((double) temp);
      counter = windowSize;
   }
   return lastOutput;
}


