/* A non-interpolating allpass filter class, by John Gibson, 1999.
*/
#if !defined(__Allpass_h)
#define __Allpass_h

#include "DLineN.h" 

class Allpass : public JGFilter
{
  protected:  
    DLineN   *delayLine;
    double allPassCoeff;
    double loopt;
  public:
    Allpass(double srate, double loopTime, double reverbTime);
    ~Allpass();
    void clear();
    void setReverbTime(double reverbTime);
    double tick(double input);
};

#endif

