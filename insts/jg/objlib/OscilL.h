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
    OscilL(double   srate,
           MY_FLOAT initialPhase,
           double   *waveTable,
           int      tableSize);
    ~OscilL();
    MY_FLOAT tick(MY_FLOAT freq, MY_FLOAT amp);
};

#endif

