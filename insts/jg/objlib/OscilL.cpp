/* An interpolating oscillator class, by John Gibson
   (based on oscili in cmix genlib and later Objective C classes)
*/

#include "OscilL.h"


/* Inherits from OscilN; see comments at OscilN constructor.
*/
OscilL :: OscilL(
   MY_FLOAT initialPhase = 0.0,
   double   *waveTable = NULL,
   int      tableSize = DEFAULT_WAVETABLE_SIZE)
        : OscilN(initialPhase, waveTable, tableSize)
{
   // nothing else to do
}


OscilL :: ~OscilL()
{
}


MY_FLOAT OscilL :: tick(MY_FLOAT freq, MY_FLOAT amp = 1.0)
{
   int    i, k;
   double frac;

   i = (int) phase;
   k = (i + 1) % size;
   frac = phase - (double) i;

   lastOutput = (table[i] + (table[k] - table[i]) * frac) * amp;

   // prepare for next call
   phase += increment * freq;
   while (phase >= (double) size)
      phase -= (double) size;
   while (phase < 0.0)
      phase += (double) size;

   return lastOutput;
}


