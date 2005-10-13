// ThreadedAudioDevice.cpp
//
// Base class for all classes which spawn thread to run loop.
//

#include "ThreadedAudioDevice.h"
#include <sys/time.h>
#include <sys/resource.h>	// setpriority()
#include <sys/select.h>
#include <string.h>			// memset()
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#define DEBUG 0

#if DEBUG > 1
#define PRINT0 if (1) printf
#define PRINT1 if (1) printf
#elif DEBUG > 0
#define PRINT0 if (1) printf
#define PRINT1 if (0) printf
#else
#define PRINT0 if (0) printf
#define PRINT1 if (0) printf
#endif

// Uncomment this to make it possible to profile RTcmix code w/ gprof
//#define PROFILE

#ifdef PROFILE
static struct itimerval globalTimerVal;
#endif

ThreadedAudioDevice::ThreadedAudioDevice()
	  : _device(-1), _thread(0), _frameCount(0),
	  _paused(false), _stopping(false), _closing(false)
{
}

int ThreadedAudioDevice::startThread()
{
	stopping(false);	// Reset.
	if (isPassive())	// Nothing else to do here if passive mode.
		return 0;
#ifdef PROFILE
	getitimer(ITIMER_PROF, &globalTimerVal);
#endif
	PRINT1("\tThreadedAudioDevice::startThread: starting thread\n");
	int status = pthread_create(&_thread, NULL, _runProcess, this);
	if (status < 0) {
		error("Failed to create thread");
	}
	return status;
}

int ThreadedAudioDevice::doStop()
{
	if (!stopping()) {
		PRINT1("\tThreadedAudioDevice::doStop\n");
		stopping(true);		// signals play thread
		paused(false);
		waitForThread();
	}
	return 0;
}

int ThreadedAudioDevice::doGetFrameCount() const
{
	return frameCount();
}

// Local method definitions

void ThreadedAudioDevice::waitForThread(int waitMs)
{
	if (!isPassive()) {
		assert(_thread != 0);	// should not get called again!
		PRINT1("ThreadedAudioDevice::waitForThread: waiting for thread to finish\n");
		if (pthread_join(_thread, NULL) == -1) {
			PRINT0("ThreadedAudioDevice::doStop: terminating thread!\n");
			pthread_cancel(_thread);
			_thread = 0;
		}
		PRINT1("\tThreadedAudioDevice::waitForThread: thread done\n");
	}
}

inline void	ThreadedAudioDevice::setFDSet()
{
	fd_set *thisSet = NULL;
#ifdef PREFER_SELECT_ON_WRITE
	if (isRecording() && !isPlaying())
		// Use read fd_set for half-duplex record only.
		thisSet = &_rfdset;
	else if (isPlaying())
		// Use write fd_set for full-duplex and half-duplex play.
		thisSet = &_wfdset;
#else
	if (isRecording())
		// Use read fd_set for for full-duplex and half-duplex record.
		thisSet = &_rfdset;
	else if (isPlaying() && !isRecording())
		// Use write fd_set for half-duplex play only.
		thisSet = &_wfdset;
#endif
	FD_SET(_device, thisSet);
}

void ThreadedAudioDevice::setDevice(int dev)
{
	_device = dev;
	if (_device > 0) {
		FD_ZERO(&_rfdset);
		FD_ZERO(&_wfdset);
		setFDSet();
	}
}

bool ThreadedAudioDevice::waitForDevice(unsigned int wTime) {
	bool ret = false;
	unsigned waitSecs = int(wTime / 1000.0);
	unsigned waitUsecs = (wTime * 1000) - unsigned(waitSecs * 1.0e+06);
	// Wait wTime msecs for select to return, then bail.
	if (!stopping()) {
		int nfds = _device + 1;
		struct timeval tv;
		tv.tv_sec = waitSecs;
		tv.tv_usec = waitUsecs;
		// If wTime == 0, wait forever by passing NULL as the final arg.
//		if (!isPlaying())
//			printf("select(%d, 0x%x, 0x%x, NULL, 0x%x)...\n", 
//					nfds, &_rfdset, &_wfdset, wTime == 0 ?  NULL : &tv);
		int selret = ::select(nfds, &_rfdset, &_wfdset,
							  NULL, wTime == 0 ?  NULL : &tv);
		if (selret <= 0) {
			if (errno != EINTR)
				fprintf(stderr,
						"ThreadedAudioDevice::waitForDevice: select %s\n",
						(selret == 0) ? "timed out" : "returned error");
			ret = false;
		}
		else {
			setFDSet();
			ret = true;
		}
	}
	else {
		PRINT1("ThreadedAudioDevice::waitForDevice: stopping == true\n");
	}
	return ret;
}

void *ThreadedAudioDevice::_runProcess(void *context)
{
#ifdef PROFILE
	setitimer(ITIMER_PROF, &globalTimerVal, NULL);
	getitimer(ITIMER_PROF, &globalTimerVal);
#endif
	ThreadedAudioDevice *device = (ThreadedAudioDevice *) context;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int status = pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_destroy(&attr);
	if (status != 0)
	{
		device->error("Failed to set scheduling policy of thread");
		return NULL;
	}
	if (setpriority(PRIO_PROCESS, 0, -20) != 0)
	{
//			perror("ThreadedAudioDevice::startThread: Failed to set priority of thread.");
	}
	device->run();
	return NULL;
}
