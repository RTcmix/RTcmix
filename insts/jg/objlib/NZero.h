/* General multiple-zero (FIR) filter class, by John Gibson, 1999
   Much code adapted from clm and snd.
*/
#if !defined(__NZero_h)
#define __NZero_h

#include "JGFilter.h"

class NZero : public JGFilter
{
  protected:  
    int     order;
    double  *zeroCoeffs;
  public:
    NZero(double srate, int ntaps);
    ~NZero();
    void clear();
    void setZeroCoeffs(double *coeffs);
    void setGain(double aValue);
    float getFrequencyResponse(float freq);
    void designFromFunctionTable(double *table, int size, double low,
                                                                  double high);
    double tick(double sample);
};

#endif
