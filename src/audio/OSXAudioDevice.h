// OSXAudioDevice.h

#ifndef _OSX_AUDIO_DEVICE_H_
#define _OSX_AUDIO_DEVICE_H_

#include "AudioDeviceImpl.h"

class OSXAudioDevice : public AudioDeviceImpl {
public:
	OSXAudioDevice();
	virtual ~OSXAudioDevice();
	
protected:
	// AudioDeviceImpl reimplementation
	virtual int doOpen(int mode);
	virtual int doClose();
	virtual int doStart();
	virtual int doPause(bool);
	virtual int doSetFormat(int sampfmt, int chans, double srate);
	virtual int doStop();
	virtual int doSetQueueSize(int *pQueueSize);
	virtual int	doGetFrames(void *frameBuffer, int frameCount);
	virtual int	doSendFrames(void *frameBuffer, int frameCount);
	virtual int doGetFrameCount() const;
private:
	struct	Impl;
	friend struct Impl;
	Impl	*_impl;
};

#endif	// _OSX_AUDIO_DEVICE_H_

