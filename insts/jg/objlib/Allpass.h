/* A non-interpolating allpass filter class, by John Gibson, 1999.
*/
#if !defined(__Allpass_h)
#define __Allpass_h

#include "DLineN.h" 

class Allpass : public Filter
{
  protected:  
    DLineN   *delayLine;
    MY_FLOAT allPassCoeff;
    MY_FLOAT loopt;
  public:
    Allpass(MY_FLOAT loopTime, MY_FLOAT reverbTime);
    ~Allpass();
    void clear();
    void setReverbTime(MY_FLOAT reverbTime);
    MY_FLOAT tick(MY_FLOAT input);
};

#endif

