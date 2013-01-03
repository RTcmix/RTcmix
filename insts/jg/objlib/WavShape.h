/* Wave shaping class, by John Gibson, 1999. Derived from cmix genlib/wshape.c.

   NOTE: transfer function table should have values in range [-1, 1], and
   samples fed to tick() should also be in this range.
*/
#if !defined(__WavShape_h)
#define __WavShape_h

#include "JGFilter.h"

class WavShape : public JGFilter
{
  protected:  
    int      lastIndex;
    int      indexFactor;
    double   *transferFunc;
  public:
    WavShape(void);
    ~WavShape();
    void setTransferFunc(double *aFunc, int aSize);
    double tick(double sample);
};

#endif
