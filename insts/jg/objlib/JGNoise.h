/* Noise Generator Class, by Perry R. Cook, 1995-96
   Returns pseudo-random numbers in the range (-1.0, 1.0).
   (Seed arg and method added by JGG.)
*/

#if !defined(__JGNoise_h)
#define __JGNoise_h

#include "objdefs.h"

class JGNoise
{
  protected:  
    double lastOutput;
  public:
    JGNoise(unsigned int aSeed = 0);
    virtual ~JGNoise();
    void seed(unsigned int aSeed);
    double tick();
    double lastOut();
};

#endif

