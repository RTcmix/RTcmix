/* BiQuad (2-pole, 2-zero) Filter Class, by Perry R. Cook, 1995-96
   See books on filters to understand more about how this works.
   Nothing out of the ordinary in this version.

   (Changed to use new/delete; added setFreqBandwidthAndGain and
   some comments.  -JGG)
*/
#if !defined(__BiQuad_h)
#define __BiQuad_h

#include "Filter.h"

class BiQuad : public Filter
{
  protected:  
    double poleCoeffs[2];
    double zeroCoeffs[2];
  public:
    BiQuad(double srate);
    ~BiQuad();
    void clear();
    void setPoleCoeffs(double *coeffs);
    void setZeroCoeffs(double *coeffs);
    void setGain(double aValue);
    void setEqualGainZeroes();
    void setFreqAndReson(double freq, double reson);
    void setFreqBandwidthAndGain(double freq, double bw, double aGain);
    double tick(double sample);
};

#endif
