/* Wave shaping class, by John Gibson, 1999. Derived from cmix genlib/wshape.c.

   NOTE: transfer function table should have values in range [-1, 1], and
   samples fed to tick() should also be in this range.
*/
#if !defined(__WavShape_h)
#define __WavShape_h

#include "Filter.h"

class WavShape : public Filter
{
  protected:  
    int      lastIndex;
    int      indexFactor;
    MY_FLOAT *transferFunc;
  public:
    WavShape(void);
    ~WavShape();
    void setTransferFunc(MY_FLOAT *aFunc, int aSize);
    MY_FLOAT tick(MY_FLOAT sample);
};

#endif
