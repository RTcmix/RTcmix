// handle.c 
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
