// AudioDevice.h -- abstract base class for system-dependent audio

#ifndef _AUDIODEVICE_H_
#define _AUDIODEVICE_H_

class AudioDevice {
public:
	typedef bool (*Callback)(AudioDevice *, void *context);
	enum {
		Unset = 0x0,
		Record = 0x1,
		Playback = 0x2,
		RecordPlayback = 0x3,
		DirectionMask = 0xf,
		Passive = 0x10
	};
public:
	virtual				~AudioDevice();
	
	// Set the format of the frames to be handed to the device.
	virtual int			setFrameFormat(int sampfmt, int chans) = 0;
	// Open the HW in mode 'mode' with suggested audio format
	virtual int			open(int mode, int sampfmt, int chans, double sr) = 0;
	virtual int			close() = 0;
	virtual int			start(Callback runCallback, void *callbackContext) = 0;
	virtual int			setStopCallback(Callback stopCallback, void *callbackContext) = 0;
	virtual int			pause(bool) = 0;
	virtual int			stop() = 0;
	virtual int			setFormat(int fmt, int chans, double srate) = 0;
	// Returns actual size via pFrames.
	virtual int			setQueueSize(int *pWriteSize, int *pCount) = 0;
	virtual int			getFrames(void *frameBuffer, int frameCount) = 0;
	virtual int			sendFrames(void *frameBuffer, int frameCount) = 0;
	virtual bool		isOpen() const = 0;
	virtual bool		isRunning() const = 0;
	virtual bool		isPaused() const = 0;
	virtual int			getFrameFormat() const = 0;
	virtual int			getDeviceFormat() const = 0;
	virtual bool		isFrameInterleaved() const = 0;
	virtual bool		isDeviceInterleaved() const = 0;
	virtual int			getFrameChannels() const = 0;
	virtual int			getDeviceChannels() const = 0;
	virtual double		getSamplingRate() const = 0;
	virtual long		getFrameCount() const = 0;
	virtual	const char *getLastError() const = 0;
protected:
	// For use by all
	enum State {
		Closed = 0,
		Open = 1,
		Configured = 2,
		Running = 3,
		Paused = 4
	};
};

// createAudioDevice must be implemented by each platform's derived AudioDevice
//	implementation code.

AudioDevice *createAudioDevice(const char *path);

#endif	// _AUDIODEVICE_H_
