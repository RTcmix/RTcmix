//	RefCounted.C -- Needed to allow locate for virtual table symbol.
//

#include <RefCounted.h>
#include <assert.h>
#ifdef USE_OSX_DISPATCH
#include <dispatch/dispatch.h>
#endif
#include <ugens.h>

RefCounted::~RefCounted()
{
#if defined(DEBUG_MEMORY) || defined(DEBUG)
	if (_refcount > 0) { rtcmix_print("Refcounted::~RefCounted(this = %p): delete called on object with nonzero ref count!\n"); assert(0); }
#endif
}

int RefCounted::unref()
{
	int r;
#if defined(DEBUG_MEMORY) || defined(DEBUG)
	if (_refcount < 0) { rtcmix_print("Refcounted::~RefCounted(this = %p): object already deleted!\n"); assert(0); }
#endif
	if ((r=--_refcount) <= 0) {
#ifdef USE_OSX_DISPATCH
        if (_dispatch) {
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                           ^{ delete this; }
            );
        }
        else
#endif
        {
            delete this;
        }
	}
	return r;
}

void RefCounted::ref(RefCounted *r)
{
	if (r)
		r->ref();
}

// This is used to unref pointers which may be NULL without needed to do the
//	null check every time.  Just call RefCounted::Unref(ptr).

int RefCounted::unref(RefCounted *r)
{
	return (r) ? r->unref() : 0;
}
