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
    OscilN(MY_FLOAT initialPhase = 0.0,
           MY_FLOAT *waveTable = NULL,
           int      tableSize = DEFAULT_WAVETABLE_SIZE);
    ~OscilN();
    MY_FLOAT tick(MY_FLOAT freq, MY_FLOAT amp = 1.0);
};

#endif

