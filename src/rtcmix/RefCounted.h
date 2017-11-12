// RefCounted.h
//
// Base class for objects which are held by reference in multiple locations.
//

#ifndef _RT_REFCOUNTED_H_
#define _RT_REFCOUNTED_H_

class RefCounted {
public:
#ifdef DEBUG_MEMORY
	virtual int ref() { return ++_refcount; }
	virtual int unref();
#else
	int ref() { return ++_refcount; }
	int unref();
#endif
	static void ref(RefCounted *r);
	static int unref(RefCounted *r);
protected:
	RefCounted(bool dispatchOnDelete=false) : _refcount(0), _dispatch(dispatchOnDelete) {}
	virtual ~RefCounted();
private:
	short _refcount;
    bool  _dispatch;
};

#endif	//	 _RT_REFCOUNTED_H_
