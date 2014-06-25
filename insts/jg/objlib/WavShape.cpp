/* Wave shaping class, by John Gibson, 1999. Derived from cmix genlib/wshape.c.

   NOTE: transfer function table should have values in range [-1, 1], and
   samples fed to tick() should also be in this range.
*/

#include "WavShape.h"


WavShape :: WavShape() : JGFilter(0)
{
   transferFunc = NULL;
}


WavShape :: ~WavShape()
{
}


/* <aFunc> is an array of <aSize> floats in the range [-1, 1].
   Caller must make sure this storage remains valid for the life of
   the object.
*/
void WavShape :: setTransferFunc(double *aFunc, int aSize)
{
   transferFunc = aFunc;
   lastIndex = aSize - 1;
   indexFactor = lastIndex / 2;
}


/* <sample> should be in range [-1, 1].
*/
double WavShape :: tick(double sample)
{
   if (transferFunc) {
      double findex = (sample + 1.0) * (double) indexFactor;
      if (findex < 0.0)
         findex = 0.0;

      int loc1 = (int) findex;
      double frac = findex - (double) loc1;

      double val1, val2;
      if (loc1 < lastIndex) {
         val1 = transferFunc[loc1];
         val2 = transferFunc[loc1 + 1];
      }
      else     // pin to last array value if sample out of range
         val2 = val1 = transferFunc[lastIndex];
      lastOutput = val1 + (frac * (val2 - val1));
   }
   else
      lastOutput = sample;

   return lastOutput;
}


