/* RMS power gauge, based on the one in csound.  -JGG */

#if !defined(__RMS_h)
#define __RMS_h

#include "Filter.h"
#include "OnePole.h"

class RMS : public Filter
{
  private:
    int      counter;
  protected:  
    int      windowSize;
    OnePole  *subLowFilter;
  public:
    RMS(double srate);
    ~RMS();
    void clear();
    void setFreq(MY_FLOAT freq);
    void setWindowSize(int nsamples);
    MY_FLOAT tick(MY_FLOAT sample);
};

#endif
