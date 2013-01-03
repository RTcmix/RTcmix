/* RMS power gauge, based on the one in csound.  -JGG */

#if !defined(__RMS_h)
#define __RMS_h

#include "JGFilter.h"
#include "JGOnePole.h"

class RMS : public JGFilter
{
  private:
    int      counter;
  protected:  
    int      windowSize;
    JGOnePole  *subLowFilter;
  public:
    RMS(double srate);
    ~RMS();
    void clear();
    void setFreq(double freq);
    void setWindowSize(int nsamples);
    double tick(double sample);
};

#endif
