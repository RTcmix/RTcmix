/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include <rtcmix_types.h>
#include <assert.h>
#include <stdio.h>

#ifdef __cplusplus
Handle createPFieldHandle(class PField *);
Handle createInstHandle(class Instrument *);
inline void refHandle(Handle h)
{
	assert(h->refcount >= 0);
    printf("refHandle(%p): %d -> ", h, h->refcount);
	++h->refcount;
    printf("%d\n", h->refcount);
}
#endif	// __cplusplus

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

void unrefHandle(Handle);

#ifdef __cplusplus
}
#endif	// __cplusplus

#ifndef __cplusplus
// This is macro-ized to allow inlining in C
#define refHandle(h) \
	{ \
	printf("refHandle(%p): %d -> ", h, h->refcount); \
	assert(h->refcount >= 0); \
	++h->refcount; \
	printf("%d\n", h->refcount); \
	} 

#endif	// !__cplusplus
