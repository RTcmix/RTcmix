/* An interpolating table lookup (one-shot oscillator) class, by John Gibson
   (This is like tablei in cmix genlib.)
*/

#include "TableL.h"


/* <duration> is the time span the table will govern, in seconds.
   <aTable> is a pointer to an array of MY_FLOAT allocated by caller.
   (Caller is responsible for ensuring that <aTable> stays valid for the
   life of this object.) <tableSize> is the length of the table.
*/
TableL :: TableL(MY_FLOAT duration, double *aTable, int tableSize)
        : TableN(duration, aTable, tableSize)
{
}


TableL :: ~TableL()
{
}


MY_FLOAT TableL :: tick(long nsample, MY_FLOAT amp = 1.0)
{
   phase = (double) nsample * increment;

   if (phase < 0.0)
      lastOutput = table[0];
   else if (phase >= (double) (size - 1))
      lastOutput = table[size - 1];
   else {
      long     index;
      double   frac, val0, val1;

      index = (long) phase;
      frac = phase - (double) index;
      val0 = table[index];
      val1 = table[index + 1];
      lastOutput = (MY_FLOAT) (val0 + frac * (val1 - val0));
   }
   lastOutput *= amp;

   return lastOutput;
}


