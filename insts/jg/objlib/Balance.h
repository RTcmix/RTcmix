/* Adjust amplitude of a signal so that it matches a comparator signal.
   Based on the one in csound.  -JGG
*/
#if !defined(__Balance_h)
#define __Balance_h

#include "JGFilter.h"
#include "RMS.h"

class Balance : public JGFilter
{
  private:
    int     counter;
    double  increment;
  protected:  
    int     windowSize;
    RMS     *inputRMS;
    RMS     *compareRMS;
  public:
    Balance(double srate);
    ~Balance();
    void clear();
    void setInitialGain(double aGain);
    void setFreq(double freq);
    void setWindowSize(int nsamples);
    double tick(double inputSamp, double compareSamp);
};

#endif
