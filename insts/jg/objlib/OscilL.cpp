/* An interpolating oscillator class, by John Gibson
   (based on oscili in cmix genlib and later Objective C classes)
*/

#include "OscilL.h"


/* Inherits from OscilN; see comments at OscilN constructor.
*/
OscilL :: OscilL(
   double   srate,
   double   initialPhase = 0.0,
   double   *waveTable = NULL,
   int      tableSize = DEFAULT_WAVETABLE_SIZE)
   : OscilN(srate, initialPhase, waveTable, tableSize)
{
}


OscilL :: ~OscilL()
{
}


double OscilL :: tick(double freq, double amp = 1.0)
{
   int i = (int) phase;
   int k = (i + 1) % size;
   double frac = phase - (double) i;

   lastOutput = (table[i] + (table[k] - table[i]) * frac) * amp;

   // prepare for next call
   phase += increment * freq;
   while (phase >= (double) size)
      phase -= (double) size;
   while (phase < 0.0)
      phase += (double) size;

   return lastOutput;
}


