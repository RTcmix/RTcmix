/* NRev Reverb Subclass
   by Tim Stilson, 1998
     based on CLM NRev
   Integrated into STK by Gary Scavone

   This is based on some of the famous Stanford CCRMA reverbs (NRev,
   KipRev) all based on the the Chowning/Moorer/Schroeder reverberators,
   which use networks of simple allpass and comb delay filters.  This
   particular arrangement consists of 6 comb filters in parallel,
   followed by 3 allpass filters, a lowpass filter, and another allpass
   in series, followed by two allpass filters in parallel with
   corresponding right and left outputs.
*/
#if !defined(__NRev_h)
#define __NRev_h

#include "Reverb.h" 
#include "DLineN.h" 

class NRev : public Reverb
{
  protected:  
    DLineN *APdelayLine[8];
    DLineN *CdelayLine[6];
    double allPassCoeff;
    double combCoef[6];
    double lpLastout;
    double lastOutL;
    double lastOutR;
    double effectMix;
  public:
    NRev(double srate, double T60);
    ~NRev();
    void clear();
    void setEffectMix(double mix);
    double lastOutput();
    double lastOutputL();
    double lastOutputR();
    double tick(double input);
};

#endif
