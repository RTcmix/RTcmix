/*******************************************/
/*  Two Zero Filter Class,                 */
/*  by Perry R. Cook, 1995-96              */ 
/*  See books on filters to understand     */
/*  more about how this works.  Nothing    */
/*  out of the ordinary in this version.   */
/*******************************************/

#if !defined(__TwoZero_h)
#define __TwoZero_h

#include "Filter.h"

class TwoZero : public Filter
{
  protected:  
    MY_FLOAT zeroCoeffs[2];
  public:
    TwoZero(double srate);
    ~TwoZero();
    void clear();
    void setZeroCoeffs(MY_FLOAT *coeffs);
    void setGain(MY_FLOAT aValue);
    void setFreqAndWidth(MY_FLOAT freq, MY_FLOAT width);
    MY_FLOAT tick(MY_FLOAT sample);
};

#endif
