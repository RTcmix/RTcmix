/* A non-interpolating IIR comb filter class, by John Gibson, 1999.
   <reverbTime> can be negative.
*/
#if !defined(__Comb_h)
#define __Comb_h

#include "DLineN.h" 

class Comb : public JGFilter
{
  protected:  
    DLineN  *delayLine;
    double  combCoeff;
    double  loopt;
  public:
    Comb(double srate, double loopTime, double reverbTime);
    ~Comb();
    void clear();
    void setReverbTime(double reverbTime);
    double tick(double input);
};

#endif

