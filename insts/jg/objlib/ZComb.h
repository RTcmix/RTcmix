/* An interpolating IIR comb filter class, by John Gibson, 1999.
   The <delaySamps> argument to tick() gives the number of samples
   of delay for that call. This number can have a fractional part.
   <reverbTime> can be negative.
*/
#if !defined(__ZComb_h)
#define __ZComb_h

#include "DLineL.h" 

class ZComb : public JGFilter
{
  protected:  
    DLineL   *delayLine;
    double combCoeff;
    double loopt;
    double delsamps;
  public:
    ZComb(double srate, double loopTime, double reverbTime);
    ~ZComb();
    void clear();
    void setReverbTime(double reverbTime);
    double tick(double input, double delaySamps);
};

#endif

