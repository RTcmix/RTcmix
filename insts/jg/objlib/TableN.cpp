/* A non-interpolating table lookup (one-shot oscillator) class, by John Gibson
   (This is like table in cmix genlib.)
*/

#include "TableN.h"


/* <duration> is the time span the table will govern, in seconds.
   <aTable> is a pointer to an array of MY_FLOAT allocated by caller.
   (Caller is responsible for ensuring that <aTable> stays valid for the
   life of this object.) <tableSize> is the length of the table.
*/
TableN :: TableN(MY_FLOAT duration, double *aTable, int tableSize) : Oscil()
{
   assert(aTable != NULL && tableSize > 0 && duration > 0.0);

   size = tableSize;
   table = aTable;
   increment = (double) size / (double) (duration * SR);
}


TableN :: ~TableN()
{
}


MY_FLOAT TableN :: tick(long nsample, MY_FLOAT amp = 1.0)
{
   long index;

   phase = (double) nsample * increment;
   index = (long) phase;

   if (index < 0)
      index = 0;
   else if (index >= size)
      index = size - 1;

   lastOutput = table[index] * amp;

   return lastOutput;
}


