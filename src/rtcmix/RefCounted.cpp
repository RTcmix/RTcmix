//	RefCounted.C -- Needed to allow locate for virtual table symbol.
//

#include <RefCounted.h>

RefCounted::~RefCounted() {}

// This is used to unref pointers which may be NULL without needed to do the
//	null check every time.  Just call RefCounted::Unref(ptr).

int RefCounted::Unref(RefCounted *r)
{
	return (r) ? r->Unref() : 0;
}
