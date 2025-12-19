/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
  the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef _RTCMIX_RWLOCK_H_
#define _RTCMIX_RWLOCK_H_

#include <pthread.h>

// Read/Write Lock class.

typedef void *RWLockHandle;

class RWLock {
public:
	RWLock(RWLockHandle h) : _handle(h), _locked(false) {}
	inline ~RWLock();
	inline void ReadLock();
	inline void WriteLock();
	inline void Unlock();
private:
	RWLockHandle _handle;
	bool _locked;
};

inline void RWLock::ReadLock()
{
	pthread_rwlock_t *rwlock = (pthread_rwlock_t *) _handle;
	pthread_rwlock_rdlock(rwlock);
	_locked = true;
}

inline void RWLock::WriteLock()
{
	pthread_rwlock_t *rwlock = (pthread_rwlock_t *) _handle;
	pthread_rwlock_wrlock(rwlock);
	_locked = true;
}

inline void RWLock::Unlock()
{
	if (_locked) {
		pthread_rwlock_t *rwlock = (pthread_rwlock_t *) _handle;
		pthread_rwlock_unlock(rwlock);
		_locked = false;
	}
}

inline RWLock::~RWLock() { Unlock(); }

#endif	// _RTCMIX_RWLOCK_H_
