// JackAudioDevice.h
//
#ifndef _JACK_AUDIODEVICE_H_
#define _JACK_AUDIODEVICE_H_

#if defined(JACK)

#include "ThreadedAudioDevice.h"

class JackAudioDevice : public ThreadedAudioDevice {
public:
	JackAudioDevice();
	virtual ~JackAudioDevice();
	
protected:
	// ThreadedAudioDevice reimplementation
	virtual void run();
	// AudioDeviceImpl reimplementation
	virtual int doOpen(int mode);
	virtual int doClose();
	virtual int doStart();
	virtual int doPause(bool);
	virtual int doSetFormat(int sampfmt, int chans, double srate);
	virtual int doSetQueueSize(int *pWriteSize, int *pCount);
	virtual int	doGetFrames(void *frameBuffer, int frameCount);
	virtual int	doSendFrames(void *frameBuffer, int frameCount);
private:
	struct Impl;
	Impl			*_impl;
};

#endif

#endif	// _JACK_AUDIODEVICE_H_

