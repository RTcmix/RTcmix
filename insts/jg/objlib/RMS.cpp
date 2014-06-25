/* RMS power gauge, based on the one in csound.  -JGG */

#include "RMS.h"


RMS :: RMS(double srate) : JGFilter(srate)
{
   gain = 1.0;
   subLowFilter = new JGOnePole(srate);
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
   subLowFilter->clear();
}


void RMS :: setFreq(double freq)
{
   subLowFilter->setFreq(freq);
}


void RMS :: setWindowSize(int nsamples)
{
   windowSize = nsamples;
   counter = 0;
}


double RMS :: tick(double sample)
{
   double temp = subLowFilter->tick(sample * sample);
   if (--counter < 0) {
      lastOutput = sqrt(temp);
      counter = windowSize;
   }
   return lastOutput;
}


