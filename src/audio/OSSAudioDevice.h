// OSSAudioDevice.h

#ifndef _OSS_AUDIO_DEVICE_H_
#define _OSS_AUDIO_DEVICE_H_

#ifndef ALSA

#include "ThreadedAudioDevice.h"

class OSSAudioDevice : public ThreadedAudioDevice {
public:
	OSSAudioDevice(const char *devPath="/dev/dsp");
	virtual ~OSSAudioDevice();
	
protected:
	// ThreadedAudioDevice reimplementation
	virtual void run();
	// AudioDeviceImpl reimplementation
	virtual int doOpen(int mode);
	virtual int doClose();
	virtual int doStart();
	virtual int doPause(bool);
	virtual int doSetFormat(int sampfmt, int chans, double srate);
	virtual int doSetQueueSize(int *pQueueSize);
	virtual int	doGetFrames(void *frameBuffer, int frameCount);
	virtual int	doSendFrames(void *frameBuffer, int frameCount);
	// Local methods
	int			ioctl(int req, void *argp);
	int			bufferSize() const { return _bufferSize; }
private:
	const char *	_inputDeviceName;
	const char *	_outputDeviceName;
	int				_bytesPerFrame;
	int				_bufferSize;
};

#endif	// !ALSA

#endif	// _OSS_AUDIO_DEVICE_H_
