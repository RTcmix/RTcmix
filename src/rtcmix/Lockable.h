// Lockable.h
//
// Base class for all classes which support mutex locking.
// Class is entirely inline, so no .C source.
//

#ifndef _RT_LOCKABLE_H_
#define _RT_LOCKABLE_H_

#include <globals.h>
#include <lock.h>

class Lockable {
public:
	Lockable() { pthread_mutex_init(&_mutex, NULL); }
	~Lockable() { pthread_mutex_destroy(&_mutex); }
	void		lock() { pthread_mutex_lock(&_mutex); }
	void		unlock() { pthread_mutex_unlock(&_mutex); }
protected:
	// This allows subclasses to create temp Lock class instances
	LockHandle	getLockHandle() { return (LockHandle) &_mutex; }
private:
	pthread_mutex_t _mutex;
};

#endif	//	 _RT_LOCKABLE_H_

