/* An interpolating oscillator class, by John Gibson
   (based on oscili in cmix genlib and later Objective C classes)
*/

#if !defined(__OscilL_h)
#define __OscilL_h

#include "OscilN.h"

class OscilL : public OscilN
{
  protected:  
  public:
    OscilL(MY_FLOAT initialPhase = 0.0,
           MY_FLOAT *waveTable = NULL,
           int      tableSize = DEFAULT_WAVETABLE_SIZE);
    ~OscilL();
    MY_FLOAT tick(MY_FLOAT freq, MY_FLOAT amp = 1.0);
};

#endif

