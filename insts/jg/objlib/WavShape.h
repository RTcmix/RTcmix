/* Wave shaping class, by John Gibson, 1999. Derived from cmix genlib/wshape.c.
*/
#if !defined(__WavShape_h)
#define __WavShape_h

#include "Filter.h"

class WavShape : public Filter
{
  protected:  
    int      lastIndex;
    int      indexFactor;
    MY_FLOAT limit;
    MY_FLOAT *transferFunc;
  public:
    WavShape(MY_FLOAT aLimit = 1.0);
    ~WavShape();
    void setTransferFunc(MY_FLOAT *aFunc, int aSize);
    MY_FLOAT tick(MY_FLOAT sample);
};

#endif
