/* Oscillator base class, by John Gibson
   (on the model of Filter.C)
*/

#include "Oscil.h"


Oscil :: Oscil(double srate) : _sr(srate)
{
}


Oscil :: ~Oscil()
{
}


// Change offset into the wave table. <aPhase> is in radians.
// Caution: Changing this when osc amplitude is non-zero can cause clicks, etc.
void Oscil :: setPhase(double aPhase)
{
   phase = aPhase * ONE_OVER_TWO_PI * (double) size;
}


double Oscil :: lastOut()
{
   return lastOutput;
}


double *getSineTable()
{
   static double *sinetab = NULL;

   if (sinetab == NULL) {
      int     size = DEFAULT_WAVETABLE_SIZE;
      double  phs, incr;

      sinetab = new double [size];
      incr = TWO_PI / (double) size;
      phs = 0.0;
      for (int i = 0; i < size; i++, phs += incr)
         sinetab[i] = sin(phs);
   }
   return sinetab;
}


