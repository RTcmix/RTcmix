/* Adjust amplitude of a signal so that it matches a comparator signal.
   Based on the one in csound.  -JGG
*/
#if !defined(__Balance_h)
#define __Balance_h

#include "Filter.h"
#include "RMS.h"

class Balance : public Filter
{
  private:
    int      counter;
    MY_FLOAT increment;
  protected:  
    int      windowSize;
    RMS      *inputRMS;
    RMS      *compareRMS;
  public:
    Balance();
    ~Balance();
    void clear();
    void setInitialGain(MY_FLOAT aGain);
    void setFreq(MY_FLOAT freq);
    void setWindowSize(int nsamples);
    MY_FLOAT tick(MY_FLOAT inputSamp, MY_FLOAT compareSamp);
};

#endif
