//
//  EmbeddedAudioDevice.h
//  RTcmix
//	Generic passive audio device designed to be called via a public callback function.
//  Created by Douglas Scott on 12/30/15.
//
//

#ifndef _EMBEDDED_AUDIODEVICE_H_
#define _EMBEDDED_AUDIODEVICE_H_

extern "C" int SetEmbeddedCallbackAudioFormat(int sampfmt, int chans);

#include "AudioDeviceImpl.h"

class EmbeddedAudioDevice : public AudioDeviceImpl
{
public:
	// Recognizer
	static bool recognize(const char *);
	// Creator
	static AudioDevice* create(const char *, const char *, int);
	// Public run routine, called from public callback
	bool run(void *inputFrameBuffer, void *outputFrameBuffer, int frameCount);
protected:
	EmbeddedAudioDevice();
	virtual ~EmbeddedAudioDevice();
	// Base class overrides.
	virtual int 	doOpen(int mode);
	virtual int 	doClose();
	virtual int 	doStart();
	virtual int 	doPause(bool);
	virtual int 	doStop();
	// Sets device format using settings as hints.
	virtual int 	doSetFormat(int sampfmt, int chans, double srate);
	// Returns actual write size and count via pWriteSize and pCount.
	virtual int 	doSetQueueSize(int *pWriteSize, int *pCount);
	// Returns number of frames recorded or played.
	virtual int 	doGetFrameCount() const;
	// Returns number of frames read, or -1 for error.
	virtual	int		doGetFrames(void *frameBuffer, int frameCount);
	// Returns number of frames written, or -1 for error.
	virtual	int		doSendFrames(void *frameBuffer, int frameCount);
private:
	struct Impl;
	Impl *_impl;
};

#endif /* defined(_EMBEDDED_AUDIODEVICE_H_) */
