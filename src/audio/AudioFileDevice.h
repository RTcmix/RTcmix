// AudioFileDevice.h
//
#ifndef _AUDIOFILEDEVICE_H_
#define _AUDIOFILEDEVICE_H_

#include "ThreadedAudioDevice.h"

class AudioFileDevice : public ThreadedAudioDevice {
public:
	enum { CheckPeaks = 1 };
	AudioFileDevice(const char *path, int fileType, int fileOptions);
	virtual ~AudioFileDevice();

	// AudioDeviceImpl overrides.
	virtual int open(int mode, int sampfmt, int chans, double srate);
	virtual int	sendFrames(void *frameBuffer, int frames);
	// New to class
	double getPeak(int chan, long *location);

protected:
    // ThreadedAudioDevice redefine.
    virtual void run();
	// AudioDeviceImpl reimplementation
	virtual int doOpen(int mode);
	virtual int doClose();
	virtual int doStart();
	virtual int doPause(bool);
	virtual int doSetFormat(int sampfmt, int chans, double srate);
	virtual int doSetQueueSize(int *pWriteSize, int *pCount);
	virtual int doGetFrameCount() const;
	virtual int doGetFrames(void *frameBuffer, int frameCount);
	virtual	int	doSendFrames(void *frameBuffer, int frameCount);
private:
	struct Impl;
	Impl	*_impl;
};

#endif	// _AUDIOFILEDEVICE_H_
