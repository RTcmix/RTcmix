/* General multiple-zero (FIR) filter class, by John Gibson, 1999
   Much code adapted from clm and snd.
*/
#if !defined(__NZero_h)
#define __NZero_h

#include "Filter.h"

class NZero : public Filter
{
  protected:  
    int      order;
    MY_FLOAT *zeroCoeffs;
  public:
    NZero(int ntaps);
    ~NZero();
    void clear();
    void setZeroCoeffs(MY_FLOAT *coeffs);
    void setGain(MY_FLOAT aValue);
    float getFrequencyResponse(float freq);
    void designFromFunctionTable(float *table, int size, float low, float high);
    MY_FLOAT tick(MY_FLOAT sample);
};

#endif
