/* Noise Generator Class, by Perry R. Cook, 1995-96
   Returns pseudo-random numbers in the range (-1.0, 1.0).
   (Seed arg and method added by JGG.)
*/

#if !defined(__Noise_h)
#define __Noise_h

#include "objdefs.h"

class Noise
{
  protected:  
    MY_FLOAT lastOutput;
  public:
    Noise(unsigned int aSeed = 0);
    virtual ~Noise();
    void seed(unsigned int aSeed = 0);
    MY_FLOAT tick();
    MY_FLOAT lastOut();
};

#endif

