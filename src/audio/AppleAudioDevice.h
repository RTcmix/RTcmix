//
//  AppleAudioDevice.h
//  Created by Douglas Scott on 11/12/13.
//
//

#ifndef _APPLE_AUDIODEVICE_H_
#define _APPLE_AUDIODEVICE_H_

#include "AudioDeviceImpl.h"

class AppleAudioDevice : public AudioDeviceImpl {
public:
	AppleAudioDevice(const char *desc=NULL);
	virtual ~AppleAudioDevice();
	// Recognizer
	static bool			recognize(const char *);
	// Creator
	static AudioDevice*	create(const char *, const char *, int);
	
protected:
	// AudioDeviceImpl reimplementation
	virtual int doOpen(int mode);
	virtual int doClose();
	virtual int doStart();
	virtual int doPause(bool);
	virtual int doSetFormat(int sampfmt, int chans, double srate);
	virtual int doStop();
	virtual int doSetQueueSize(int *pWriteSize, int *pCount);
	virtual int	doGetFrames(void *frameBuffer, int frameCount);
	virtual int	doSendFrames(void *frameBuffer, int frameCount);
	virtual int doGetFrameCount() const;
	
	virtual int getRecordDeviceChannels() const;
	virtual int getPlaybackDeviceChannels() const;
	
	void		parseDeviceDescription(const char *inDesc);
private:
	struct	Impl;
	friend struct Impl;
	Impl	*_impl;
};

#endif /* defined(_APPLE_AUDIODEVICE_H_) */
