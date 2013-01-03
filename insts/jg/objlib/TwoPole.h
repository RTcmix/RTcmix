/*******************************************/
/*  Two Pole Filter Class,                 */
/*  by Perry R. Cook, 1995-96              */ 
/*  See books on filters to understand     */
/*  more about how this works.  Nothing    */
/*  out of the ordinary in this version.   */
/*******************************************/

#if !defined(__TwoPole_h)
#define __TwoPole_h

#include "JGFilter.h"

class TwoPole : public JGFilter
{
  protected:  
    double poleCoeffs[2];
  public:
    TwoPole(double srate);
    ~TwoPole();
    void clear();
    void setPoleCoeffs(double *coeffs);
    void setGain(double aValue);
    void setFreqAndReson(double freq, double reson);
    void setFreqBandwidthAndScale(double freq, double bw, int scale);
    double tick(double sample);
};

#endif
