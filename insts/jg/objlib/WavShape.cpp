/* Wave shaping class, by John Gibson, 1999. Derived from cmix genlib/wshape.c.
*/

#include "WavShape.h"


WavShape :: WavShape(MY_FLOAT aLimit = 1.0)
{
   limit = aLimit;
   transferFunc = NULL;
}


WavShape :: ~WavShape()
{
}


/* <aFunc> is an array of <aSize> floats in the range [-1, 1].
   Caller must make sure this storage remains valid for the life of
   the object.
*/
void WavShape :: setTransferFunc(MY_FLOAT *aFunc, int aSize)
{
   transferFunc = aFunc;
   lastIndex = aSize - 1;
   indexFactor = lastIndex / 2;
}


MY_FLOAT WavShape :: tick(MY_FLOAT sample)
{
   int      loc1;
   MY_FLOAT findex, frac, val1, val2;

   if (transferFunc) {
      findex = (sample + limit) * indexFactor;
      if (findex < 0.0)
         findex = 0.0;

      loc1 = (int)findex;
      frac = findex - (MY_FLOAT)loc1;
      if (loc1 < lastIndex) {
         val1 = transferFunc[loc1];
         val2 = transferFunc[loc1 + 1];
      }
      else     // pin to last array value if sample out of range
         val2 = val1 = transferFunc[lastIndex];
      lastOutput = val1 + frac * (val2 - val1);
   }
   else
      lastOutput = sample;

   return lastOutput;
}


