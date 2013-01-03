#if !defined(__Butter_h)
#define __Butter_h

#include "JGFilter.h"

class Butter : public JGFilter
{
  private:
    double c, d;
    int type;
  protected:
    double poleCoeffs[2];
    double zeroCoeffs[2];
  public:
    Butter(double srate);
    ~Butter();
    void clear();
    void setLowPass(double cutoff);
    void setHighPass(double cutoff);
    void setBandPassFreq(double freq);
    void setBandPassBandwidth(double bandwidth);
    void setBandPass(double freq, double bandwidth);
    void setBandRejectFreq(double freq);
    void setBandRejectBandwidth(double bandwidth);
    void setBandReject(double freq, double bandwidth);
    double tick(double sample);
};

#endif
