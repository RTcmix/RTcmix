/* A non-interpolating IIR comb filter class, by John Gibson, 1999.
   <reverbTime> can be negative.
*/
#if !defined(__Comb_h)
#define __Comb_h

#include "DLineN.h" 

class Comb : public Filter
{
  protected:  
    DLineN   *delayLine;
    MY_FLOAT combCoeff;
    MY_FLOAT loopt;
  public:
    Comb(MY_FLOAT loopTime, MY_FLOAT reverbTime);
    ~Comb();
    void clear();
    void setReverbTime(MY_FLOAT reverbTime);
    MY_FLOAT tick(MY_FLOAT input);
};

#endif

