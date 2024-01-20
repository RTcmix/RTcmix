/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// utils.cpp
// Functions for creating Handles and tables
//

#include "handle.h"
#include <PField.h>
#include <stdlib.h>
#include <assert.h>
#include <ugens.h>

Handle
createPFieldHandle(PField *pfield)
{
	Handle handle = (Handle) malloc(sizeof(struct _handle));
	if (handle) {
		handle->type = PFieldType;
		handle->ptr = (void *) pfield;
#ifdef DEBUG_MEMORY
		rtcmix_print("\tcreated PField Handle %p\n", handle);
    	rtcmix_print("\t\trefing PField %p\n", pfield);
#endif
		pfield->ref();
		handle->refcount = 0;
	}
	return handle;
}

Handle
createInstHandle(Instrument *inst)
{
	Handle handle = (Handle) malloc(sizeof(struct _handle));
	if (handle) {
		handle->type = InstrumentPtrType;
		handle->ptr = (void *) inst;
#ifdef DEBUG_MEMORY
		rtcmix_print("\tcreated Instrument Handle %p\n", handle);
    	rtcmix_print("\t\t (not refing) inst = %p\n", inst);
#endif
		handle->refcount = 0;
	}
	return handle;
}

Handle
createArrayHandle(Array *array)
{
    Handle handle = (Handle) malloc(sizeof(struct _handle));
    if (handle) {
        handle->type = ListType;        // indicates we are returning an object to be referenced as a list
        handle->ptr = (void *) array;
        handle->refcount = 0;
    }
    return handle;
}

void refHandle(Handle h)
{
	assert(h->refcount >= 0);
#ifdef DEBUG_MEMORY
    rtcmix_print("refHandle(%p): %d -> %d\n", h, h->refcount, h->refcount+1);
#endif
	++h->refcount;
}

void
unrefHandle(Handle h)
{
#ifdef DEBUG_MEMORY
    rtcmix_print("unrefHandle(%p): %d -> %d\n", h, h->refcount, h->refcount-1);
#endif
	assert(h->refcount >= 0);
	--h->refcount;
    if (h->refcount == 0)
    {
#ifdef DEBUG_MEMORY
        rtcmix_print("\tfreeing Handle %p\n", h);
		h->refcount = 0xdeaddead;	// for debugging
#endif
		if (h->type == PFieldType) {
#ifdef DEBUG_MEMORY
			rtcmix_print("\tunrefing PField %p\n", h->ptr);
#endif
			RefCounted::unref((RefCounted *)h->ptr);
		}
		else if (h->type == InstrumentPtrType) {
#ifdef DEBUG_MEMORY
			rtcmix_print("\tnot yet unrefing inst %p\n", h->ptr);
#endif
		}
		else {
			rtcmix_warn("unrefHandle", "unrefHandle: unknown handle type!");
			return;
		}
        free(h);
    }
}	


