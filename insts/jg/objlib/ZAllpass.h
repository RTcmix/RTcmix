/* A non-interpolating allpass filter class, by John Gibson, 1999.
*/
#if !defined(__ZAllpass_h)
#define __ZAllpass_h

#include "DLineL.h" 

class ZAllpass : public JGFilter
{
  protected:  
    DLineL   *delayLine;
    double allPassCoeff;
    double loopt;
    double delsamps;
  public:
    ZAllpass(double srate, double loopTime, double reverbTime);
    ~ZAllpass();
    void clear();
    void setReverbTime(double reverbTime);
    double tick(double input, double delaySamps);
};

#endif

