/* Biquad equalizer class, based on code by Tom St Denis
   <tomstdenis.home.dhs.org>, which in turn is based on
   the Cookbook formulae for audio EQ biquad filter coefficients
   by Robert Bristow-Johnson. (Google for "Audio-EQ-Cookbook".)

   Reimplemented by John Gibson, 12/7/03.
*/
#if !defined(__Equalizer_h)
#define __Equalizer_h

#include "objdefs.h"

#ifndef M_LN2
   #define M_LN2  0.69314718055994530942
#endif  

typedef enum {
   EQLowPass = 0,
   EQHighPass,
   EQBandPassCSG,    // CSG: constant skirt gain; peak gain = Q
   EQBandPassCPG,    // CPG: constant 0 dB peak gain
   EQBandPass = EQBandPassCPG,
   EQNotch,
   EQAllPass,
   EQPeaking,
   EQLowShelf,
   EQHighShelf,
   EQInvalid
} EQType;

class Equalizer
{
protected:  
   double   _sr;
   double   c0, c1, c2, c3, c4;
   double   x1, x2, y1, y2;
   EQType   type;
public:
   Equalizer(double srate, EQType eqType);
   ~Equalizer();
   void clear();

   void setEQType(EQType eqType) { type = eqType; }
   void setCoeffs(double freq, double Q, double gain);

   double tick(double sample)
   {
      double y0 = (c0 * sample) + (c1 * x1) + (c2 * x2)
                                - (c3 * y1) - (c4 * y2);
      x2 = x1;
      x1 = sample;
      y2 = y1;
      y1 = y0;

      return y0;
   }
    
};

#endif
