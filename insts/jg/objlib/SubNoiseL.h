/* Linear interpolating sub-sampled noise generator class, by John Gibson, 1999
   Gets new random numbers every <subSample> samples, and interpolates between
   the last and previous random numbers on each tick(). Returns numbers in the
   range (-1.0, 1.0).

   Note: This is like randi in other packages, but without amp scaling
   in tick and with freq specied in samples rather than Hz.  -JGG
*/
#if !defined(__SubNoiseL_h)
#define __SubNoiseL_h

#include "SubNoise.h"

class SubNoiseL : public SubNoise
{
  protected:
    MY_FLOAT prevRand;
    MY_FLOAT curVal;
    MY_FLOAT increment;
  public:
    SubNoiseL();
    ~SubNoiseL();
    SubNoiseL(int subSample, unsigned int aSeed = 0);
    MY_FLOAT tick();
    MY_FLOAT lastOut();
};

#endif
