/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <Oequalizer.h>
#include <math.h>
#include <assert.h>

#ifndef cosf
	#define cosf(theta) cos((theta))
#endif
#ifndef sinf
	#define sinf(theta) sin((theta))
#endif
#ifndef powf
	#define powf(val, exp) pow((val), (exp))
#endif
#ifndef sqrtf
	#define sqrtf(val) sqrt((val))
#endif

Oequalizer::Oequalizer(float srate, OeqType type)
	: _sr(srate), _type(type), 
	  _c0(0), _c1(0), _c2(0), _c3(0), _c4(0),
	  _x1(0), _x2(0), _y1(0), _y2(0)
{
#ifdef ANTI_DENORM
	_antidenorm_offset = 1e-18;	// smallest value observed to be effective
#endif
}

void Oequalizer::setparams(
   float freq,				// in Hz
   float Q,					// roughly 0.5 to 10
   float gain)				// in dB (peaking and shelving EQs only)
{
   float a0, a1, a2, b0, b1, b2, A, alpha, beta, omega, sn, cs;

   a0 = a1 = a2 = b0 = b1 = b2 = 0.0f;  // suppress init warnings

   omega = 2.0f * M_PI * (freq / _sr);
   sn = sinf(omega);
   cs = cosf(omega);

   //printf("freq=%f, Q=%f, gain=%f, _sr=%f\n", freq, Q, gain, _sr);

   switch (_type) {

      case OeqLowPass:
         alpha = sn / (2.0f * Q);

         b0 = (1.0f - cs) / 2.0f;
         b1 = 1.0f - cs;
         b2 = b0;
         a0 = 1.0f + alpha;
         a1 = -2.0f * cs;
         a2 = 1.0f - alpha;
         break;

      case OeqHighPass:
         alpha = sn / (2.0f * Q);

         b0 = (1.0f + cs) / 2.0f;
         b1 = -(1.0f + cs);
         b2 = b0;
         a0 = 1.0f + alpha;
         a1 = -2.0f * cs;
         a2 = 1.0f - alpha;
         break;

      case OeqBandPassCSG:
         alpha = sn / (2.0f * Q);

         b0 = sn / 2.0f;
         b1 = 0.0f;
         b2 = -b0;
         a0 = 1.0f + alpha;
         a1 = -2.0f * cs;
         a2 = 1.0f - alpha;
         break;

      case OeqBandPassCPG:
         alpha = sn / (2.0f * Q);

         b0 = alpha;
         b1 = 0.0f;
         b2 = -alpha;
         a0 = 1.0f + alpha;
         a1 = -2.0f * cs;
         a2 = 1.0f - alpha;
         break;

      case OeqNotch:
         alpha = sn / (2.0f * Q);

         b0 = 1.0f;
         b1 = -2.0f * cs;
         b2 = 1.0f;
         a0 = 1.0f + alpha;
         a1 = b1;
         a2 = 1.0f - alpha;
         break;

      case OeqAllPass:
         alpha = sn / (2.0f * Q);

         b0 = 1.0f - alpha;
         b1 = -2.0f * cs;
         b2 = 1.0f + alpha;
         a0 = b2;
         a1 = b1;
         a2 = b0;
         break;

      case OeqPeaking:
         alpha = sn / (2.0f * Q);
         A = powf(10.0f, gain / 40.0f);

         b0 = 1.0f + (alpha * A);
         b1 = -2.0f * cs;
         b2 = 1.0f - (alpha * A);
         a0 = 1.0f + (alpha / A);
         a1 = b1;
         a2 = 1.0f - (alpha / A);
         break;

      case OeqLowShelf:
         A = powf(10.0f, gain / 40.0f);
         beta = sqrtf(A) / Q;     // St Denis has sqrt(A + A)

         b0 = A * ((A + 1.0f) - ((A - 1.0f) * cs) + (beta * sn));
         b1 = 2.0f * A * ((A - 1.0f) - ((A + 1.0f) * cs));
         b2 = A * ((A + 1.0f) - ((A - 1.0f) * cs) - (beta * sn));
         a0 = (A + 1.0f) + ((A - 1.0f) * cs) + (beta * sn);
         a1 = -2.0f * ((A - 1.0f) + ((A + 1.0f) * cs));
         a2 = (A + 1.0f) + ((A - 1.0f) * cs) - (beta * sn);
         break;

      case OeqHighShelf:
         A = powf(10.0f, gain / 40.0f);
         beta = sqrtf(A) / Q;     // St Denis has sqrt(A + A)

         b0 = A * ((A + 1.0f) + ((A - 1.0f) * cs) + (beta * sn));
         b1 = -2.0f * A * ((A - 1.0f) + ((A + 1.0f) * cs));
         b2 = A * ((A + 1.0f) + ((A - 1.0f) * cs) - (beta * sn));
         a0 = (A + 1.0f) - ((A - 1.0f) * cs) + (beta * sn);
         a1 = 2.0f * ((A - 1.0f) - ((A + 1.0f) * cs));
         a2 = (A + 1.0f) - ((A - 1.0f) * cs) - (beta * sn);
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

