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
    OscilN(MY_FLOAT initialPhase,
           MY_FLOAT *waveTable,
           int      tableSize);
    ~OscilN();
    MY_FLOAT tick(MY_FLOAT freq, MY_FLOAT amp);
};

#endif

