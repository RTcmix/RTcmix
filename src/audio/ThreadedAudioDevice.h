// ThreadedAudioDevice.h
//
// Intermediate base class for devices which run a thread for the audio loop.
//

#ifndef _THREADEDAUDIODEVICE_H_
#define _THREADEDAUDIODEVICE_H_

#include "AudioDeviceImpl.h"
#include <pthread.h>
#include <sys/select.h>

class ThreadedAudioDevice : public AudioDeviceImpl {
protected:
	ThreadedAudioDevice();
	
	// Local methods
	virtual int			startThread();
	virtual void		run() = 0;
	
	// AudioDeviceImpl reimplementation
	int			doStop();
	int			doGetFrameCount() const;

	// Virtual methods new to this class.
	virtual bool waitForDevice(unsigned int wTime=0);	// wTime in msec.
	
	// Utilities.
	void		waitForThread(int waitMs=1000);
	void		setDevice(int dev);
	int			device() const { return _device; }
	int			frameCount() const { return _frameCount; }
	void		resetFrameCount() { _frameCount = 0; }
	void		incrementFrameCount(int frames) { _frameCount += frames; }
	// These are to avoid reentrant code.
	void		paused(bool p) { _paused = p; }
	void		stopping(bool s) { _stopping = s; }
	void		closing(bool c) { _closing = c; }
	bool		paused() const { return _paused; }
	bool		stopping() const { return _stopping; }
	bool		closing() const { return _closing; }
	static void *		_runProcess(void *);
private:
	int			_device;
	fd_set		_fdset;
	pthread_t	_thread;
	int			_frameCount;
	bool		_paused;
	bool		_stopping;
	bool		_closing;
};

#endif	// _THREADEDAUDIODEVICE_H_
