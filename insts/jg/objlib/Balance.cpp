/* Adjust amplitude of a signal so that it matches a comparator signal.
   Based on the one in csound.  -JGG
*/
#include "Balance.h"


Balance :: Balance(double srate) : JGFilter(srate)
{
   outputs = inputs = NULL;       // unused in this class
   inputRMS = new RMS(_sr);
   compareRMS = new RMS(_sr);
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


void Balance :: setInitialGain(double aGain)
{
   gain = aGain;
}


void Balance :: setFreq(double freq)
{
   inputRMS->setFreq(freq);
   compareRMS->setFreq(freq);
}


void Balance :: setWindowSize(int nsamples)
{
   assert(nsamples > 0);          // doesn't seem to work if nsamples is zero

   windowSize = nsamples;
   counter = windowSize + 1;      // sync with RMS "if (--counter < 0)" blocks
   inputRMS->setWindowSize(windowSize);
   compareRMS->setWindowSize(windowSize);
}


double Balance :: tick(double inputSamp, double compareSamp)
{
   double in, cmp;

   // Note: must maintain rms histories even when we don't consult in and cmp
   in = inputRMS->tick(inputSamp);
   cmp = compareRMS->tick(compareSamp);

   if (--counter < 0) {
      double a, diff;

      a  = in ? cmp / in : cmp;
      diff = a - gain;
      increment = diff / windowSize;

      counter = windowSize;
   }

   lastOutput = inputSamp * gain;
   gain += increment;

   return lastOutput;
}


