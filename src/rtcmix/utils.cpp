/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// utils.cpp
// Functions for creating Handles and tables
//

#include <utils.h>
#include <PField.h>
#include <stdlib.h>
#include <assert.h>

Handle
createPFieldHandle(PField *pfield)
{
	Handle handle = (Handle) malloc(sizeof(struct _handle));
	if (handle) {
		handle->type = PFieldType;
		handle->ptr = (void *) pfield;
#ifdef DEBUG
    	printf("\trefing PField %p\n", pfield);
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
		handle->refcount = 0;
	}
	return handle;
}

void
unrefHandle(Handle h)
{
	assert(h->refcount >= 0);
#ifdef DEBUG
    printf("unrefHandle(%p): %d -> ", h, h->refcount);
#endif
	--h->refcount;
#ifdef DEBUG
    printf("%d\n", h->refcount);
#endif
    if (h->refcount == 0 && h->type == PFieldType)
    {
#ifdef DEBUG
        printf("\tfreeing Handle %p\n", h);
		h->refcount = 0xdeaddead;	// for debugging
        printf("\tunrefing PField %p\n", h->ptr);
#endif
		RefCounted::unref((PField *)h->ptr);
        free(h);
    }
}	


