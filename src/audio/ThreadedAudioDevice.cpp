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
//	printf("\tThreadedAudioDevice::startThread: starting thread\n");
	int status = pthread_create(&_thread, NULL, _runProcess, this);
	if (status == 0) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		status = pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_attr_destroy(&attr);
		if (status != 0)
		{
			return error("Failed to set scheduling policy of thread");
		}
		if (setpriority(PRIO_PROCESS, 0, -20) != 0)
		{
//			perror("ThreadedAudioDevice::startThread: Failed to set priority of thread.");
		}
	}
	return status;
}

int ThreadedAudioDevice::doStop()
{
	if (!stopping()) {
//		printf("\tThreadedAudioDevice::doStop: waiting for thread to finish\n");
		stopping(true);		// signals play thread
		paused(false);
		if (!isPassive())
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
	if (pthread_join(_thread, NULL) == -1) {
//		printf("ThreadedAudioDevice::doStop: terminating thread!\n");
		pthread_cancel(_thread);
		_thread = 0;
	}
//	printf("\tThreadedAudioDevice::waitForThread: thread done\n");
}

void ThreadedAudioDevice::setDevice(int dev)
{
	_device = dev;
	if (_device > 0) {
		FD_ZERO(&_fdset);
		FD_SET(_device, &_fdset);
	}
}

bool ThreadedAudioDevice::waitForDevice(unsigned int wTime) {
	bool ret = false;
	unsigned waitSecs = int(wTime / 1000.0);
	unsigned waitUsecs = (wTime * 1000) - unsigned(waitSecs * 1.0e+06);
	// Wait wTime msecs for select to return, then bail.
	if (!stopping()) {
		struct timeval tv;
		tv.tv_sec = waitSecs;
		tv.tv_usec = waitUsecs;
		// If wTime == 0, wait forever by passing NULL as the final arg.
		int selret = ::select(_device + 1, NULL, &_fdset, 
							  NULL, wTime == 0 ?  NULL : &tv);
		if (selret <= 0) {
			fprintf(stderr,
					"ThreadedAudioDevice::waitForDevice: select %s\n",
					(selret == 0) ? "timed out" : "returned error");
			ret = false;
		}
		else {
			FD_SET(_device, &_fdset);
			ret = true;
		}
	}
	return ret;
}

void *ThreadedAudioDevice::_runProcess(void *context)
{
	ThreadedAudioDevice *device = (ThreadedAudioDevice *) context;
	device->run();
	return NULL;
}
