/* Biquad equalizer class, based on code by Tom St Denis
   <tomstdenis.home.dhs.org>, which in turn is based on
   the Cookbook formulae for audio EQ biquad filter coefficients
   by Robert Bristow-Johnson. (Google for "Audio-EQ-Cookbook".)

   Reimplemented by John Gibson, 12/7/03.
*/

#include "Equalizer.h"


Equalizer :: Equalizer(double srate, EQType eqType)
   : _sr(srate), type(eqType)
{
   c0 = c1 = c2 = c3 = c4 = 0.0;
   x1 = x2 = y1 = y2 = 0.0;
}


Equalizer :: ~Equalizer()
{
}


void Equalizer :: clear()
{
   x1 = x2 = y1 = y2 = 0.0;
}


void Equalizer :: setCoeffs(
   double freq,          // in Hz
   double Q,             // roughly 0.5 to 10
   double gain = 0.0)    // in dB (peaking and shelving EQs only)
{
   double a0, a1, a2, b0, b1, b2, A, alpha, beta, omega, sn, cs;

   a0 = a1 = a2 = b0 = b1 = b2 = 0.0;  // suppress init warnings

   omega = 2.0 * M_PI * (freq / _sr);
   sn = sin(omega);
   cs = cos(omega);

   //printf("freq=%f, Q=%f, gain=%f, _sr=%f\n", freq, Q, gain, _sr);

   switch (type) {

      case EQLowPass:
         alpha = sn / (2.0 * Q);

         b0 = (1.0 - cs) / 2.0;
         b1 = 1.0 - cs;
         b2 = b0;
         a0 = 1.0 + alpha;
         a1 = -2.0 * cs;
         a2 = 1.0 - alpha;
         break;

      case EQHighPass:
         alpha = sn / (2.0 * Q);

         b0 = (1.0 + cs) / 2.0;
         b1 = -(1.0 + cs);
         b2 = b0;
         a0 = 1.0 + alpha;
         a1 = -2.0 * cs;
         a2 = 1.0 - alpha;
         break;

      case EQBandPassCSG:
         alpha = sn / (2.0 * Q);

         b0 = sn / 2.0;
         b1 = 0.0;
         b2 = -b0;
         a0 = 1.0 + alpha;
         a1 = -2.0 * cs;
         a2 = 1.0 - alpha;
         break;

      case EQBandPassCPG:
         alpha = sn / (2.0 * Q);

         b0 = alpha;
         b1 = 0.0;
         b2 = -alpha;
         a0 = 1.0 + alpha;
         a1 = -2.0 * cs;
         a2 = 1.0 - alpha;
         break;

      case EQNotch:
         alpha = sn / (2.0 * Q);

         b0 = 1.0;
         b1 = -2.0 * cs;
         b2 = 1.0;
         a0 = 1.0 + alpha;
         a1 = b1;
         a2 = 1.0 - alpha;
         break;

      case EQAllPass:
         alpha = sn / (2.0 * Q);

         b0 = 1.0 - alpha;
         b1 = -2.0 * cs;
         b2 = 1.0 + alpha;
         a0 = b2;
         a1 = b1;
         a2 = b0;
         break;

      case EQPeaking:
         alpha = sn / (2.0 * Q);
         A = pow(10.0, gain / 40.0);

         b0 = 1.0 + (alpha * A);
         b1 = -2.0 * cs;
         b2 = 1.0 - (alpha * A);
         a0 = 1.0 + (alpha / A);
         a1 = b1;
         a2 = 1.0 - (alpha / A);
         break;

      case EQLowShelf:
         A = pow(10.0, gain / 40.0);
         beta = sqrt(A) / Q;     // St Denis has sqrt(A + A)

         b0 = A * ((A + 1.0) - ((A - 1.0) * cs) + (beta * sn));
         b1 = 2.0 * A * ((A - 1.0) - ((A + 1.0) * cs));
         b2 = A * ((A + 1.0) - ((A - 1.0) * cs) - (beta * sn));
         a0 = (A + 1.0) + ((A - 1.0) * cs) + (beta * sn);
         a1 = -2.0 * ((A - 1.0) + ((A + 1.0) * cs));
         a2 = (A + 1.0) + ((A - 1.0) * cs) - (beta * sn);
         break;

      case EQHighShelf:
         A = pow(10.0, gain / 40.0);
         beta = sqrt(A) / Q;     // St Denis has sqrt(A + A)

         b0 = A * ((A + 1.0) + ((A - 1.0) * cs) + (beta * sn));
         b1 = -2.0 * A * ((A - 1.0) + ((A + 1.0) * cs));
         b2 = A * ((A + 1.0) + ((A - 1.0) * cs) - (beta * sn));
         a0 = (A + 1.0) - ((A - 1.0) * cs) + (beta * sn);
         a1 = 2.0 * ((A - 1.0) - ((A + 1.0) * cs));
         a2 = (A + 1.0) - ((A - 1.0) * cs) - (beta * sn);
         break;

      case EQInvalid:
         assert("Equalizer::setCoeffs: invalid EQ type" && 0);
         break;
   }

   c0 = b0 / a0;
   c1 = b1 / a0;
   c2 = b2 / a0;
   c3 = a1 / a0;
   c4 = a2 / a0;
}

