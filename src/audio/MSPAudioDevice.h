//
//  MSPAudioDevice.h
//  RTcmix
//
//  Created by Douglas Scott on 12/2/13.
//
//

#ifndef _MSP_AUDIODEVICE_H_
#define _MSP_AUDIODEVICE_H_

#include "AudioDeviceImpl.h"

class MSPAudioDevice : public AudioDeviceImpl {
public:
	MSPAudioDevice(const char *path);
	virtual ~MSPAudioDevice();
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
	virtual int doStop();
	virtual int doSetFormat(int sampfmt, int chans, double srate);
	virtual int doSetQueueSize(int *pWriteSize, int *pCount);
	virtual int doGetFrameCount() const;
	virtual int	doGetFrames(void *frameBuffer, int frameCount);
	virtual int	doSendFrames(void *frameBuffer, int frameCount);
	
protected:
	friend struct Impl;
private:
	struct Impl;
	Impl			*_impl;
};


#endif /* defined(_MSP_AUDIODEVICE_H_) */
