/* An interpolating IIR comb filter class, by John Gibson, 1999.
   The <delaySamps> argument to tick() gives the number of samples
   of delay for that call. This number can have a fractional part.
   <reverbTime> can be negative.
*/
#if !defined(__ZComb_h)
#define __ZComb_h

#include "DLineL.h" 

class ZComb : public Filter
{
  protected:  
    DLineL   *delayLine;
    MY_FLOAT combCoeff;
    MY_FLOAT loopt;
    MY_FLOAT delsamps;
  public:
    ZComb(MY_FLOAT loopTime, MY_FLOAT reverbTime);
    ~ZComb();
    void clear();
    void setReverbTime(MY_FLOAT reverbTime);
    MY_FLOAT tick(MY_FLOAT input, MY_FLOAT delaySamps);
};

#endif

