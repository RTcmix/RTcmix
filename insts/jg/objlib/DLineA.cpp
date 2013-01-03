/* AllPass Interpolating Delay Line Object by Perry R. Cook 1995-96
   This one uses a delay line of maximum length specified on creation,
   and interpolates fractional length using an all-pass filter.  This
   version is more efficient for computing static length delay lines
   (alpha and coeff are computed only when the length is set, there
   probably is a more efficient computational form if alpha is changed
   often (each sample)).
*/

#include "DLineA.h"


DLineA :: DLineA(long max_length) : JGFilter(0)
{
   length = max_length;
   inputs = new double [length];
   this->clear();
   inPoint = 0;
   outPoint = length >> 1;
}


DLineA :: ~DLineA()
{
   delete [] inputs;
}


void DLineA :: clear()
{
   for (long i = 0; i < length; i++)
      inputs[i] = 0.0;
   lastIn = 0.0;
   lastOutput = 0.0;
}


void DLineA :: setDelay(double lag)  
{
   double outputPointer;

   outputPointer = inPoint - lag + 2;         // outPoint chases inpoint
                                              //   + 2 for interp and other
   while (outputPointer < 0)                                               
      outputPointer += length;                // modulo table length
   outPoint = (long) outputPointer;           // integer part of delay
   alpha = 1.0 + outPoint - outputPointer;    // fractional part of delay
   if (alpha < 0.1) {
      outputPointer += 1.0;                   // hack to avoid pole/zero
      outPoint += 1;                          // cancellation.  Keeps allpass
      alpha += 1.0;                           // delay in range of .1 to 1.1
   }
   coeff = (1.0 - alpha) / (1.0 + alpha);     // coefficient for all pass
}


double DLineA :: tick(double sample)          // take sample, yield sample
{
   double temp;

   inputs[inPoint++] = sample;                // write input sample
   if (inPoint == length)                     // increment input pointer
      inPoint -= length;                      // modulo length
   temp = inputs[outPoint++];                 // filter input
   if (outPoint == length)                    // increment output pointer
      outPoint -= length;                     // modulo length
   lastOutput = -coeff * lastOutput;          // delayed output
   lastOutput += lastIn + (coeff * temp);     // input + delayed input
   lastIn = temp;
   return lastOutput;                         // save output and return
}


