/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
// Special interface code to allow reference counting on opaque handles

#include <minc_internal.h>
#include "handle.h"
#include "../../rtcmix/utils.h"

void
ref_handle(MincHandle h)
{
	Handle handle = (Handle) h;
	if (handle)
		refHandle(handle);
}

void
unref_handle(MincHandle h)
{
	Handle handle = (Handle) h;
	if (handle)
		unrefHandle(handle);
}
