/* Oscillator base class, by John Gibson
   (on the model of Filter.C)
*/

#include "Oscil.h"


Oscil :: Oscil()
{
}


Oscil :: ~Oscil()
{
}


// Change offset into the wave table. <aPhase> is in radians.
// Caution: Changing this when osc amplitude is non-zero can cause clicks, etc.
void Oscil :: setPhase(MY_FLOAT aPhase)
{
   phase = (double) (aPhase * ONE_OVER_TWO_PI * (double) size);
}


MY_FLOAT Oscil :: lastOut()
{
   return lastOutput;
}


MY_FLOAT *getSineTable()
{
   static MY_FLOAT *sinetab = NULL;

   if (sinetab == NULL) {
      int     size = DEFAULT_WAVETABLE_SIZE;
      double  phs, incr;

      sinetab = new MY_FLOAT [size];
      incr = TWO_PI / (double) size;
      phs = 0.0;
      for (int i = 0; i < size; i++, phs += incr)
         sinetab[i] = (MY_FLOAT) sin(phs);
   }
   return sinetab;
}


