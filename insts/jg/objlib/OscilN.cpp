/* A non-interpolating oscillator class, by John Gibson
   (based on Cmix genlib and later Objective C classes)
*/

#include "OscilN.h"


/* Construct OscilN object. Uses default args unless provided with others.
   <initialPhase> is the initial phase offset into the wave table, given
   in radians. <waveTable> is a pointer to an array of double allocated
   by caller. (Caller is responsible for ensuring that <waveTable> stays
   valid for the life of this object.) Or, if <waveTable> is NULL, an
   internal sine wave table (of one cycle) will be used. If caller provides
   <waveTable>, <tableSize> must give the length of the table.
   Examples:

      // uses internal sine table, initial phase = 0
      myosc = new OscilN(srate);

      // uses internal sine table, initial phase = 90 deg (i.e., cosine)
      myosc = new OscilN(srate, PI/2);

      // uses myTable of size elements, init. phase = 0 (note: must give phase)
      myosc = new OscilN(srate, 0, myTable, size);
*/
OscilN :: OscilN(
   double   srate,
   double   initialPhase = 0.0,
   double   *waveTable = NULL,
   int      tableSize = DEFAULT_WAVETABLE_SIZE)
   : Oscil(srate)
{
   if (waveTable) {
      size = tableSize;
      table = waveTable;
   }
   else {
      size = DEFAULT_WAVETABLE_SIZE;
      table = getSineTable();           // get ptr to internal sine table
   }
   increment = (double) size / _sr;
   phase = initialPhase * ONE_OVER_TWO_PI * (double) size;
}


OscilN :: ~OscilN()
{
}


double OscilN :: tick(double freq, double amp = 1.0)
{
   int i = (int) phase;
   lastOutput = table[i] * amp;

   // prepare for next call
   phase += increment * freq;
   while (phase >= (double) size)
      phase -= (double) size;
   while (phase < 0.0)
      phase += (double) size;

   return lastOutput;
}


