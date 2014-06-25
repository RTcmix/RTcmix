/* Linearly Interpolating Delay Line Object by Perry R. Cook 1995-96
   This one uses a delay line of maximum length specified on creation,
   and linearly interpolates fractional length. It is designed to be
   more efficient if the delay length is not changed very often.

   JGG added alternative API, to be used in place of setDelay / tick.
   Useful mainly for situations where you want multiple taps, which you
   can't do with the other API. To summarize the two API's:

    (1) setDelay / tick:
          setDelay(lag);
          output = tick(input);     // output is <lag> samps older than input

    (2) putSample / getSample:
          putSample(input);         // like tick, but no output returned
          output = getSample(lag);  // output is lag samps older than last input
          out2 = getSample(lag2);   // can call again before next putSample

   The <lag> argument for setDelay and getSample can have a fractional
   part, in which case linear interpolation determines the output from
   the delay line.

   Note that in the interest of efficiency, putSample and getSample do
   not maintain the correct values for outPoint and omAlpha. (If you want
   to use the first API after having used the second one for a DLineL
   object, call setDelay to init outPoint and omAlpha.)
*/
#include "DLineL.h"


DLineL :: DLineL(long max_length) : JGFilter(0)
{
   /* Add 2 to let user call setDelay(max_length) next without worrying
      about outPoint stepping on inPoint.  -JGG
   */
   length = max_length + 2;       

   inputs = new double [length];
   this->clear();
   outPoint = 0;
   inPoint = length >> 1;                // by default, .5 of max delay  -JGG
}


DLineL :: ~DLineL()
{
   delete [] inputs;
}


void DLineL :: clear()
{
   for (long i = 0; i < length; i++)
      inputs[i] = 0.0;
   lastOutput = 0.0;
}


void DLineL :: setDelay(double lag)
{
   double outputPointer;

   outputPointer = inPoint - lag;       // read chases write
   while (outputPointer < 0.0)
      outputPointer += length;          // modulo maximum length
   outPoint = (long) outputPointer;     // integer part
   alpha = outputPointer - outPoint;    // fractional part
   omAlpha = 1.0 - alpha;               // 1 - fract. part (more efficient)
}


double DLineL :: tick(double input)              // take one, yield one
{
   inputs[inPoint++] = input;                    // input next sample
   if (inPoint == length)                        // check for end condition
      inPoint -= length;
   lastOutput = inputs[outPoint++] * omAlpha;    // first 1/2 of interpolation
   if (outPoint < length)                        // check for end condition
      lastOutput += inputs[outPoint] * alpha;    // second 1/2 of interpolation
   else {                                        // if at end ...
      lastOutput += inputs[0] * alpha;           // second 1/2 of interpolation
      outPoint -= length;                             
   }
   return lastOutput;
}


/* Like tick, but doesn't bother maintaining outPoint. Use this in
   conjunction with getSample only!
   If you want to use tick again, precede it by a call to setDelay
   to reinitialize outPoint.
*/
void DLineL :: putSample(double input)
{
   inputs[inPoint++] = input;                    // input next sample
   if (inPoint == length)                        // check for end condition
      inPoint -= length;
}


/* Get sample from delay line that is <lag> samples behind the most recent
   sample to enter the delay line. If <lag> is longer than length of delay
   line, it wraps around. (So check first.)
   (Note: this is like the cmix genlib dliget, except it takes the number
   of samples of delay rather than a delay time in seconds.)
*/
double DLineL :: getSample(double lag)
{
   double outputPointer;

   outputPointer = inPoint - lag;
   while (outputPointer < 0.0)
      outputPointer += (double) length;
   outPoint = (long) outputPointer;
   alpha = outputPointer - outPoint;
   lastOutput = inputs[outPoint++];
   if (outPoint < length)
      lastOutput += (inputs[outPoint] - lastOutput) * alpha;
   else
      lastOutput += (inputs[0] - lastOutput) * alpha;

   return lastOutput;
}


