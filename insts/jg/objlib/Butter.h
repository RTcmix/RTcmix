#if !defined(__Butter_h)
#define __Butter_h

#include "Filter.h"

class Butter : public Filter
{
  private:
    double c, d;
    int type;
  protected:
    MY_FLOAT poleCoeffs[2];
    MY_FLOAT zeroCoeffs[2];
  public:
    Butter(double srate);
    ~Butter();
    void clear();
    void setLowPass(MY_FLOAT cutoff);
    void setHighPass(MY_FLOAT cutoff);
    void setBandPassFreq(MY_FLOAT freq);
    void setBandPassBandwidth(MY_FLOAT bandwidth);
    void setBandPass(MY_FLOAT freq, MY_FLOAT bandwidth);
    void setBandRejectFreq(MY_FLOAT freq);
    void setBandRejectBandwidth(MY_FLOAT bandwidth);
    void setBandReject(MY_FLOAT freq, MY_FLOAT bandwidth);
    MY_FLOAT tick(MY_FLOAT sample);
};

#endif
