/* A non-interpolating oscillator class, intended to be called
   at a control rate.
   (by JGG, based on Eric Lyon's similar Objective C class for cmix)

   This works just like the OscilN object, but has an additional
   mandatory argument for the constructor, <howOften>. Set this to
   the number of audio-rate samples you'll skip between calls to tick.
*/

#include "KOscilN.h"


KOscilN :: KOscilN(
   int      howOften,
   MY_FLOAT initialPhase = 0.0,
   MY_FLOAT *waveTable = NULL,
   int      tableSize = DEFAULT_WAVETABLE_SIZE)
         : OscilN(initialPhase, waveTable, tableSize)
{
   increment *= (MY_FLOAT) howOften;       // compensate for samples we'll skip
}


KOscilN :: ~KOscilN()
{
}


