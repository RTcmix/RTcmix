/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

// utils.h
// Decls of Handle and other utilities.

#include "rtcmix_types.h"
#include <assert.h>
#include <stdio.h>

#ifdef __cplusplus
Handle createPFieldHandle(class PField *);
Handle createInstHandle(class Instrument *);
extern "C" {
#endif	// __cplusplus
	void refHandle(Handle h);
	void unrefHandle(Handle h);
#ifdef __cplusplus
}
#endif	// __cplusplus
