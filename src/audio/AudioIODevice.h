// AudioIODevice.h -- allows independent input and output audio device 
//					  classes to function as one full-duplex device.

#ifndef _AUDIOIODEVICE_H_
#define _AUDIOIODEVICE_H_

#include "AudioDevice.h"

class AudioIODevice : public AudioDevice {
public:
	AudioIODevice(AudioDevice *inputDevice, AudioDevice *outputDevice);
	virtual ~AudioIODevice();
	// AudioDevice redefinitions.
	virtual int			setFrameFormat(int sampfmt, int chans);
	virtual int			open(int mode, int sampfmt, int chans, double sr);
	virtual int			close();
	virtual int			start(Callback runCallback, void *callbackContext);
	virtual int			setStopCallback(Callback stopCallback, void *callbackContext);
	virtual int			pause(bool);
	virtual int			stop();
	virtual int			setFormat(int fmt, int chans, double srate);
	virtual int			setQueueSize(int *pWriteSize, int *pCount);
	virtual int			getFrames(void *frameBuffer, int frameCount);
	virtual int			sendFrames(void *frameBuffer, int frameCount);
	virtual bool		isOpen() const;
	virtual bool		isRunning() const;
	virtual bool		isPaused() const;
	virtual	const char *getLastError() const;

private:
	AudioDevice	*_inputDevice;
	AudioDevice *_outputDevice;
};

#endif	// _AUDIOIODEVICE_H_
