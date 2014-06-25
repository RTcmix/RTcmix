/* DC Blocking Filter, by Perry R. Cook, 1995-96
   This guy is very helpful in, uh, blocking DC.  Needed because a simple
   low-pass reflection filter allows DC to build up inside recursive
   structures.
   (changed to use new/deleted; got rid of redundant outputs[].  -JGG)
*/

#include "DCBlock.h"


DCBlock :: DCBlock() : JGFilter(0)
{
   inputs = new double [1];
   outputs = NULL;               // unused
   this->clear();
}


DCBlock :: ~DCBlock()
{
   delete [] inputs;
}


void DCBlock :: clear()
{
   inputs[0] = 0.0;
   lastOutput = 0.0;
}


double DCBlock :: tick(double sample)    
{
   lastOutput = sample - inputs[0] + (0.99 * lastOutput);
   inputs[0] = sample;
   return lastOutput;
}

