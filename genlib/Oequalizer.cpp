/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <Oequalizer.h>
#include <math.h>
#include <assert.h>

Oequalizer::Oequalizer(float SR, OeqType type)
	: _sr(SR), _type(type), 
	  _c0(0), _c1(0), _c2(0), _c3(0), _c4(0),
	  _x1(0), _x2(0), _y1(0), _y2(0)
{
}

void Oequalizer::setparams(
   float freq,				// in Hz
   float Q,					// roughly 0.5 to 10
   float gain)				// in dB (peaking and shelving EQs only)
{
   double a0, a1, a2, b0, b1, b2, A, alpha, beta, omega, sn, cs;

   a0 = a1 = a2 = b0 = b1 = b2 = 0.0;  // suppress init warnings

   omega = 2.0 * M_PI * (freq / _sr);
   sn = sin(omega);
   cs = cos(omega);

   //printf("freq=%f, Q=%f, gain=%f, _sr=%f\n", freq, Q, gain, _sr);

   switch (_type) {

      case OeqLowPass:
         alpha = sn / (2.0 * Q);

         b0 = (1.0 - cs) / 2.0;
         b1 = 1.0 - cs;
         b2 = b0;
         a0 = 1.0 + alpha;
         a1 = -2.0 * cs;
         a2 = 1.0 - alpha;
         break;

      case OeqHighPass:
         alpha = sn / (2.0 * Q);

         b0 = (1.0 + cs) / 2.0;
         b1 = -(1.0 + cs);
         b2 = b0;
         a0 = 1.0 + alpha;
         a1 = -2.0 * cs;
         a2 = 1.0 - alpha;
         break;

      case OeqBandPassCSG:
         alpha = sn / (2.0 * Q);

         b0 = sn / 2.0;
         b1 = 0.0;
         b2 = -b0;
         a0 = 1.0 + alpha;
         a1 = -2.0 * cs;
         a2 = 1.0 - alpha;
         break;

      case OeqBandPassCPG:
         alpha = sn / (2.0 * Q);

         b0 = alpha;
         b1 = 0.0;
         b2 = -alpha;
         a0 = 1.0 + alpha;
         a1 = -2.0 * cs;
         a2 = 1.0 - alpha;
         break;

      case OeqNotch:
         alpha = sn / (2.0 * Q);

         b0 = 1.0;
         b1 = -2.0 * cs;
         b2 = 1.0;
         a0 = 1.0 + alpha;
         a1 = b1;
         a2 = 1.0 - alpha;
         break;

      case OeqAllPass:
         alpha = sn / (2.0 * Q);

         b0 = 1.0 - alpha;
         b1 = -2.0 * cs;
         b2 = 1.0 + alpha;
         a0 = b2;
         a1 = b1;
         a2 = b0;
         break;

      case OeqPeaking:
         alpha = sn / (2.0 * Q);
         A = pow(10.0, gain / 40.0);

         b0 = 1.0 + (alpha * A);
         b1 = -2.0 * cs;
         b2 = 1.0 - (alpha * A);
         a0 = 1.0 + (alpha / A);
         a1 = b1;
         a2 = 1.0 - (alpha / A);
         break;

      case OeqLowShelf:
         A = pow(10.0, gain / 40.0);
         beta = sqrt(A) / Q;     // St Denis has sqrt(A + A)

         b0 = A * ((A + 1.0) - ((A - 1.0) * cs) + (beta * sn));
         b1 = 2.0 * A * ((A - 1.0) - ((A + 1.0) * cs));
         b2 = A * ((A + 1.0) - ((A - 1.0) * cs) - (beta * sn));
         a0 = (A + 1.0) + ((A - 1.0) * cs) + (beta * sn);
         a1 = -2.0 * ((A - 1.0) + ((A + 1.0) * cs));
         a2 = (A + 1.0) + ((A - 1.0) * cs) - (beta * sn);
         break;

      case OeqHighShelf:
         A = pow(10.0, gain / 40.0);
         beta = sqrt(A) / Q;     // St Denis has sqrt(A + A)

         b0 = A * ((A + 1.0) + ((A - 1.0) * cs) + (beta * sn));
         b1 = -2.0 * A * ((A - 1.0) + ((A + 1.0) * cs));
         b2 = A * ((A + 1.0) + ((A - 1.0) * cs) - (beta * sn));
         a0 = (A + 1.0) - ((A - 1.0) * cs) + (beta * sn);
         a1 = 2.0 * ((A - 1.0) - ((A + 1.0) * cs));
         a2 = (A + 1.0) - ((A - 1.0) * cs) - (beta * sn);
         break;

      case OeqInvalid:
         assert("Oequalizer::setparams: invalid EQ type" && 0);
         break;
   }

   _c0 = b0 / a0;
   _c1 = b1 / a0;
   _c2 = b2 / a0;
   _c3 = a1 / a0;
   _c4 = a2 / a0;
}

