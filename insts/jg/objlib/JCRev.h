/* JCRev Reverb Subclass by Tim Stilson, 1998
   based on CLM JCRev Integrated into STK by Gary Scavone

   This is based on some of the famous Stanford CCRMA reverbs (NRev,
   KipRev) all based on the the Chowning/Moorer/Schroeder reverberators,
   which use networks of simple allpass and comb delay filters.  This
   particular arrangement consists of 3 allpass filters in series,
   followed by 4 comb filters in parallel, an optional lowpass filter,
   and two decorrelation delay lines in parallel at the output.
*/

#if !defined(__JCRev_h)
#define __JCRev_h

#include "Reverb.h"
#include "DLineN.h" 

class JCRev : public Reverb
{
  protected:  
    DLineN *APdelayLine[3];
    DLineN *CdelayLine[4];
    DLineN *outLdelayLine;
    DLineN *outRdelayLine;
    double allPassCoeff;
    double combCoeff[4];
    double combsum, combsum1, combsum2;
    double lastOutL;
    double lastOutR;
    double effectMix;
  public:
    JCRev(double srate, double T60);
    ~JCRev();
    void clear();
    void setEffectMix(double mix);
    double lastOutput();
    double lastOutputL();
    double lastOutputR();
    double tick(double input);
};

#endif

