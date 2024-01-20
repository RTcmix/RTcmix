/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

// handle.h
// Decls of Handle and other utilities.

#include "rtcmix_types.h"
#include <assert.h>
#include <stdio.h>

#ifdef __cplusplus
Handle createPFieldHandle(class PField *);
Handle createInstHandle(class Instrument *);
Handle createArrayHandle(Array *array);
extern "C" {
#endif	// __cplusplus
	void refHandle(Handle h);
	void unrefHandle(Handle h);
#ifdef __cplusplus
}
#endif	// __cplusplus
