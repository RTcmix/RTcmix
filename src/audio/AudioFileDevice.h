// AudioFileDevice.h
//
#ifndef _AUDIOFILEDEVICE_H_
#define _AUDIOFILEDEVICE_H_

#include "ThreadedAudioDevice.h"

class AudioFileDevice : public ThreadedAudioDevice {
public:
	enum { NormalizeFloats = 1, CheckPeaks = 2 };
	AudioFileDevice(const char *path, int fileType, int fileFmt, int fileOptions);
	virtual ~AudioFileDevice();

	// AudioDeviceImpl override.
	virtual int open(int mode, int sampfmt, int chans, double srate);
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
	virtual int doSetQueueSize(int *pQueueSize);
	virtual int doGetFrameCount() const;
	virtual int doGetFrames(void *frameBuffer, int frameCount);
	virtual	int	doSendFrames(void *frameBuffer, int frameCount);
	// New to class
	void		*convertSamples(void *buffer, int frames);
private:
	struct Impl;
	Impl	*_impl;
};

#endif	// _AUDIOFILEDEVICE_H_
