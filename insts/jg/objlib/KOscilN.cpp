/* A non-interpolating oscillator class, intended to be called
   at a control rate.
   (by JGG, based on Eric Lyon's similar Objective C class for cmix)

   This works just like the OscilN object, but has an additional
   mandatory argument for the constructor, <howOften>. Set this to
   the number of audio-rate samples you'll skip between calls to tick.
*/

#include "KOscilN.h"


KOscilN :: KOscilN(
   double   srate,
   int      howOften,
   double   initialPhase = 0.0,
   double   *waveTable = NULL,
   int      tableSize = DEFAULT_WAVETABLE_SIZE)
   : OscilN(srate, initialPhase, waveTable, tableSize)
{
   increment *= (double) howOften;       // compensate for samples we'll skip
}


KOscilN :: ~KOscilN()
{
}


