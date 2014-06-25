/* An interpolating FIR comb filter class, by John Gibson, 1999.
   <scaler> scales the feed-forward term. It can be negative. 
   The <delaySamps> argument to tick() gives the number of samples
   of delay for that call. This number can have a fractional part.
*/
#if !defined(__ZNotch_h)
#define __ZNotch_h

#include "DLineL.h" 

class ZNotch : public JGFilter
{
  protected:  
    DLineL   *_dline;
    double _coef;
    double _delsamps;
  public:
    ZNotch(double srate, double loopTime, double scaler);
    ~ZNotch();
    void clear();
    void setScaler(double scaler);
    double tick(double input, double delaySamps);
};

#endif

