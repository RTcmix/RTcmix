/* A non-interpolating oscillator class, by John Gibson
   (based on Cmix genlib and later Objective C classes)
*/

#if !defined(__OscilN_h)
#define __OscilN_h

#include "Oscil.h"

class OscilN : public Oscil
{
  protected:  
  public:
    OscilN(double   srate,
           double   initialPhase,
           double   *waveTable,
           int      tableSize);
    ~OscilN();
    double tick(double freq, double amp);
};

#endif

