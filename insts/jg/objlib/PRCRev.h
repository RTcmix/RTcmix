/* PRCRev, a simple reverb unit by Perry Cook, 1996.
   Incorporated into the Reverb superclass by Gary Scavone, 1998.

   This is based on some of the famous Stanford CCRMA reverbs (NRev,
   KipRev) all based on the the Chowning/Moorer/Schroeder reverberators,
   which use networks of simple allpass and comb delay filters.  This
   particular structure consists of 2 allpass units in series followed
   by 2 comb filters in parallel.
*/
#if !defined(__PRCRev_h)
#define __PRCRev_h

#include "Reverb.h" 
#include "DLineN.h" 

class PRCRev : public Reverb
{
  protected:  
    DLineN *APdelayLine[2];
    DLineN *CdelayLine[2];
    double allPassCoeff;
    double combCoeff[2];
    double lastOutL;
    double lastOutR;
    double effectMix;
  public:
    PRCRev(double srate, double T60);
    ~PRCRev();
    void clear();
    void setEffectMix(double mix);
    double lastOutput();
    double lastOutputL();
    double lastOutputR();
    double tick(double input);
};

#endif

