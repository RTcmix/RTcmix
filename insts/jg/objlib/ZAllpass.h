/* A non-interpolating allpass filter class, by John Gibson, 1999.
*/
#if !defined(__ZAllpass_h)
#define __ZAllpass_h

#include "DLineL.h" 

class ZAllpass : public Filter
{
  protected:  
    DLineL   *delayLine;
    MY_FLOAT allPassCoeff;
    MY_FLOAT loopt;
    MY_FLOAT delsamps;
  public:
    ZAllpass(double srate, MY_FLOAT loopTime, MY_FLOAT reverbTime);
    ~ZAllpass();
    void clear();
    void setReverbTime(MY_FLOAT reverbTime);
    MY_FLOAT tick(MY_FLOAT input, MY_FLOAT delaySamps);
};

#endif

