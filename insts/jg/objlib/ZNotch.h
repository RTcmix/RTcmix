/* An interpolating FIR comb filter class, by John Gibson, 1999.
   <scaler> scales the feed-forward term. It can be negative. 
   The <delaySamps> argument to tick() gives the number of samples
   of delay for that call. This number can have a fractional part.
*/
#if !defined(__ZNotch_h)
#define __ZNotch_h

#include "DLineL.h" 

class ZNotch : public Filter
{
  protected:  
    DLineL   *delayLine;
    MY_FLOAT combCoeff;
    MY_FLOAT delsamps;
  public:
    ZNotch(MY_FLOAT loopTime, MY_FLOAT scaler);
    ~ZNotch();
    void clear();
    void setScaler(MY_FLOAT scaler);
    MY_FLOAT tick(MY_FLOAT input, MY_FLOAT delaySamps);
};

#endif

