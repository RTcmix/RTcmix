/* An interpolating table lookup (one-shot oscillator) class, by John Gibson
   (This is like tablei in cmix genlib.)
*/

#include "TableL.h"


/* <duration> is the time span the table will govern, in seconds.
   <aTable> is a pointer to an array of double allocated by caller.
   (Caller is responsible for ensuring that <aTable> stays valid for the
   life of this object.) <tableSize> is the length of the table.
*/
TableL :: TableL(double srate, double duration, double *aTable, int tableSize)
        : TableN(srate, duration, aTable, tableSize)
{
}


TableL :: ~TableL()
{
}


double TableL :: tick(long nsample, double amp = 1.0)
{
   phase = (double) nsample * increment;

   if (phase < 0.0)
      lastOutput = table[0];
   else if (phase >= (double) (size - 1))
      lastOutput = table[size - 1];
   else {
      long index = (long) phase;
      double frac = phase - (double) index;
      double val0 = table[index];
      double val1 = table[index + 1];
      lastOutput = val0 + (frac * (val1 - val0));
   }
   lastOutput *= amp;

   return lastOutput;
}


