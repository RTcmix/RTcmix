/* A non-interpolating oscillator class, intended to be called
   at a control rate.
   (by JGG, based on Eric Lyon's similar Objective C class for cmix)

   This works just like the OscilN object, but has an additional
   mandatory argument for the constructor, <howOften>. Set this to
   the number of audio-rate samples you'll skip between calls to tick.
*/

#if !defined(__KOscilN_h)
#define __KOscilN_h

#include "OscilN.h"

class KOscilN : public OscilN
{
  protected:  
  public:
    KOscilN(double   srate,
            int      howOften,  // caller will tick() us every <howOften> samps
            double   initialPhase,
            double   *waveTable,
            int      tableSize);
    ~KOscilN();
};

#endif

