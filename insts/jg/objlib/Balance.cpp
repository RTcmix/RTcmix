/* Adjust amplitude of a signal so that it matches a comparator signal.
   Based on the one in csound.  -JGG
*/
#include "Balance.h"


Balance :: Balance() : Filter()
{
   outputs = inputs = NULL;       // unused in this class
   inputRMS = new RMS;
   compareRMS = new RMS;
   windowSize = DEFAULT_CONTROL_RATE;
   counter = windowSize + 1;      // sync with RMS "if (--counter < 0)" blocks
   increment = 0.0;
   gain = 1.0;
}


Balance :: ~Balance()    
{
   delete inputRMS;
   delete compareRMS;
}


void Balance :: clear()
{
   inputRMS->clear();
   compareRMS->clear();
   counter = 0;
   gain = 1.0;
   increment = 0.0;
   lastOutput = 0.0;
}


void Balance :: setInitialGain(MY_FLOAT aGain)
{
   gain = aGain;
}


void Balance :: setFreq(MY_FLOAT freq)
{
   inputRMS->setFreq(freq);
   compareRMS->setFreq(freq);
}


void Balance :: setWindowSize(int nsamples)
{
   windowSize = nsamples;
   counter = windowSize + 1;      // sync with RMS "if (--counter < 0)" blocks
   inputRMS->setWindowSize(windowSize);
   compareRMS->setWindowSize(windowSize);
}


MY_FLOAT Balance :: tick(MY_FLOAT inputSamp, MY_FLOAT compareSamp)
{
   MY_FLOAT in, cmp;

   // Note: must maintain rms histories even when we don't consult in and cmp
   in = inputRMS->tick(inputSamp);
   cmp = compareRMS->tick(compareSamp);

   if (--counter < 0) {
      MY_FLOAT a, diff;

      a  = in ? cmp / in : cmp;
      diff = a - gain;
      increment = diff / windowSize;

      counter = windowSize;
   }

   lastOutput = inputSamp * gain;
   gain += increment;

   return lastOutput;
}


