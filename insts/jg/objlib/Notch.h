/* A non-interpolating FIR comb filter class, by John Gibson, 1999.
   <scaler> scales the feed-forward term. It can be negative. The effect
   is most pronounced when the scaler is 1.0 or -1.0, but is still
   much more subtle than that obtained with the IIR comb.
*/
#if !defined(__Notch_h)
#define __Notch_h

#include "DLineN.h" 

class Notch : public JGFilter
{
  protected:  
    DLineN  *delayLine;
    double  combCoeff;
  public:
    Notch(double srate, double loopTime, double scaler);
    ~Notch();
    void clear();
    void setScaler(double scaler);
    double tick(double input);
};

#endif

