#if !defined(__ClampDenormals_h)
#define __ClampDenormals_h

// Macro for killing denormalled numbers
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// Based on IS_DENORMAL macro by Jon Watte
// This code is public domain
//
// Trivially revised by John Gibson, 7 Jan, 2004
// This macro only works for i386.  Note that even for i386,
// you can disable this macro by defining DISABLE_CLAMP_DENORMALS
// before including objlib.h.
//
// Note also the DENORMAL_CHECK define in RTcmix main.C, which lets
// you raise a SIGFPE signal on detection of denormals.

#if !defined(i386)
   #define DISABLE_CLAMP_DENORMALS
#endif

#ifndef DISABLE_CLAMP_DENORMALS
   #define CLAMP_DENORMALS(sample) \
      if (((*(unsigned int *) &(sample)) & 0x7f800000) == 0) \
         (sample) = 0.0
#else
   #define CLAMP_DENORMALS(sample)
#endif

#endif   // __ClampDenormals_h
