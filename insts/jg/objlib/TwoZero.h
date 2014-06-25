/*******************************************/
/*  Two Zero Filter Class,                 */
/*  by Perry R. Cook, 1995-96              */ 
/*  See books on filters to understand     */
/*  more about how this works.  Nothing    */
/*  out of the ordinary in this version.   */
/*******************************************/

#if !defined(__TwoZero_h)
#define __TwoZero_h

#include "JGFilter.h"

class TwoZero : public JGFilter
{
  protected:  
    double zeroCoeffs[2];
  public:
    TwoZero(double srate);
    ~TwoZero();
    void clear();
    void setZeroCoeffs(double *coeffs);
    void setGain(double aValue);
    void setFreqAndWidth(double freq, double width);
    double tick(double sample);
};

#endif
