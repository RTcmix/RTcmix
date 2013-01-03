/* General N-Zero (FIR) Filter Class, by John Gibson, 1999
*/

#include "NZero.h"

// ***This should be moved to a utility file
double *resample_functable(double *table, int oldsize, int newsize);


NZero :: NZero(double srate, int ntaps) : JGFilter(srate)
{
   order = ntaps;
   inputs = new double [order];
   zeroCoeffs = new double [order];
   gain = 1.0;
   outputs = NULL;                       // unused
   this->clear();
}


NZero :: ~NZero()
{
   delete [] inputs;
   delete [] zeroCoeffs;
}


void NZero :: clear()
{
   for (int i = 0; i < order; i++)
      inputs[i] = 0.0;
   lastOutput = 0.0;
}


void NZero :: setZeroCoeffs(double *coeffs)
{
   for (int i = 0; i < order; i++)
      zeroCoeffs[i] = coeffs[i];
}


void NZero :: setGain(double aValue)
{
   gain = aValue;
}


/* Given a frequency, returns gain for that frequency when passed
   through this filter. Frequency is specified as a percentage of
   Nyquist, from 0.0 to 1.0. Gain is from 0.0 (full attenuation)
   to 1.0 (no attenuation).
   (Adapted from Snd: frequency_response() in snd-chn.c.)
*/
float NZero :: getFrequencyResponse(float freq)
{
   int   n2;
   float am, at;

   n2 = order >> 1;
   am = (order + 1) * 0.5;
   at = 0.0;
   for (int i = 0; i < n2; i++) {
      float f = (float)cos((double)(PI * (am - (float)i - 1.0) * freq));
      at += (float)zeroCoeffs[i] * f;
   }
   if (at < 0.0)
      return (-2.0 * at);
   return (2.0 * at);
}


/* Accepts a function table of <oldsize> elements, describing a series of
   line segments. Resamples this table using linear interpolation, so that
   its shape is described by <newsize> elements. Returns a new table,
   allocated with new, containing this resampled data.
*/
double *
resample_functable(double *table, int oldsize, int newsize)
{
   double *newtable = new double [newsize];

   if (newsize == oldsize) {                  // straight copy
      for (int i = 0; i < newsize; i++)
         newtable[i] = table[i];
   }
   else {
      double incr = (double) oldsize / (double) newsize;
      double f = 0.0;
      for (int i = 0; i < newsize; i++) {
         int n = (int) f;
         double frac = f - (double) n;
         double diff = 0.0;
         if (frac) {
            double next = (n + 1 < oldsize) ? table[n + 1] : table[oldsize - 1];
            diff = next - table[n];
         }
         newtable[i] = table[n] + (diff * frac);
         f += incr;
      }
   }
   return newtable;
}


/* Design FIR from a graph representing desired frequency response. The
   higher the order of the filter, the closer the design can come to the
   desired response.

   The graph is in the form of function table <table> of <size> elements,
   interpreted as evenly dividing the spectrum between <low> Hz and <high>
   Hz (inclusive). Each value is the desired amplitude (from 0.0 to 1.0)
   for its frequency bin. If <high> is 0, the upper bound will instead be
   the Nyquist frequency.

   Basic idea explained in Dodge (2nd ed.), pp 205-8. Code adapted from Snd
   (make_filter in snd-dac.c). See also CLM mus.lisp for design-FIR-from-env.
*/
void NZero :: designFromFunctionTable(double *table,
                                      int    size,
                                      double low,
                                      double high)
{
   int    i;
   double nyquist = _sr / 2.0;
   double *newtable;

   assert(table != NULL && size > 1 && low >= 0.0 && high >= 0.0);

   if (high == 0.0)
      high = nyquist;
   if (low == high) {
      fprintf(stderr,
              "designFromFunctionTable: low == high! Setting to full range.\n");
      low = 0.0;
      high = nyquist;
   }
   if (low > high) {
      fprintf(stderr,
              "designFromFunctionTable: low > high! Setting high to Nyq.\n");
      high = nyquist;
   }

   /* Get a new table of <order> elements, with evenly sampled freq
      response. If low-high range covers only part of the spectrum,
      first make a temporary table covering the whole spectrum, and
      insert <table> into this. Set values below <low> to first value
      in <table>; set values above <high> to last value in <table>.
   */
   if (low != 0.0 || high != nyquist) {
      int newsize = (int) (size * (nyquist / (high - low)) + 0.5);
      int insertat = (int) (newsize * (low / nyquist) + 0.5);

      double *tmptab = new double [newsize];

      double first = table[0];
      double last = table[size - 1];

      double *tp = tmptab;
      for (i = 0; i < insertat; i++)
         *tp++ = first;
      for (i = 0; i < size; i++)
         *tp++ = table[i];
      for ( ; i < newsize - insertat; i++)
         *tp++ = last;

      newtable = resample_functable(tmptab, newsize, (order + 1) / 2);
      delete [] tmptab;
   }
   else
      newtable = resample_functable(table, size, (order + 1) / 2);

   int n = order;
   int points = (n + 1) / 2;
   double am = 0.5 * (n + 1);
   double q = TWO_PI / (double) n;
   int jj;
   for (int j = 0, jj = n - 1; j < points; j++, jj--) {
      double xt = newtable[0] * 0.5;
      for (i = 1; i < points; i++)
         xt += (newtable[i] * cos(q * (am - j - 1) * i));
      zeroCoeffs[j] = 2.0 * xt / (double) n;
      zeroCoeffs[jj] = zeroCoeffs[j];       // symmetrical around middle coeff
   }

   delete [] newtable;
}


// could be made more efficient by implementing circular queue?

double NZero :: tick(double sample)
{
   lastOutput = 0.0;

   for (int i = order - 1; i > 0; i--) {
      lastOutput += zeroCoeffs[i] * inputs[i];
      inputs[i] = inputs[i - 1];
   }
   inputs[0] = sample;
   lastOutput += zeroCoeffs[0] * sample;

   return lastOutput;
}


