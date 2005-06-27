// Macro for killing denormalled numbers
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// Based on IS_DENORMAL macro by Jon Watte
// This code is public domain

#ifndef _denormals_
#define _denormals_

#if defined(i386)

#ifdef NOMORE	// original code doesn't work on recent gcc compilers
#define undenormalise(sample) \
	if (((*(unsigned int*)&sample)&0x7f800000) == 0) sample = 0.0f

#else // !NOMORE

// see <ccrma-mail.stanford.edu/pipermail/planetccrma/2005-January/007868.html>

static inline float undenormalise(volatile float s)
{
	s += 9.8607615E-32f;
	return s - 9.8607615E-32f;
}

//#define undenormalise(sample)
#endif // !NOMORE

#else // !defined(i386)
#define undenormalise(sample) // nothing
#endif // !defined(i386)

#endif//_denormals_

//ends
