// AudioDeviceImpl.h -- partial implementation for basic devices

#ifndef _RT_AUDIODEVICEIMPL_H_
#define _RT_AUDIODEVICEIMPL_H_

#include <sndlibsupport.h>	// RTcmix header
#include "AudioDevice.h"

typedef void *(ConversionFunction)(void *, void*, int, int);

class AudioDeviceImpl : public AudioDevice {
public:
	// Redefined from AudioDevice
	int				setFrameFormat(int sampfmt, int chans);
	int				open(int mode, int sampfmt, int chans, double srate);
	int				close();
	int				start(Callback runCallback, void *callbackContext);
	int				setStopCallback(Callback stopCallback, void *callbackContext);
	int				pause(bool);
	int				stop();
	int				setFormat(int sampfmt, int chans, double srate);
	int				setQueueSize(int *pWriteSize, int *pCount);
	int				getFrames(void *frameBuffer, int frameCount);
	int				sendFrames(void *frameBuffer, int frameCount);
	bool			isOpen() const;
	bool			isRunning() const;
	bool			isPaused() const;
	const char *	getLastError() const;

protected:
	inline int		getFrameFormat() const;
	inline int		getDeviceFormat() const;
	inline bool		isFrameFmtNormalized() const;
	inline bool		isDeviceFmtNormalized() const;
	inline bool		isFrameInterleaved() const;
	inline bool		isDeviceInterleaved() const;
	inline int		getFrameChannels() const;
	inline int		getDeviceChannels() const;
	inline double	getSamplingRate() const;
	inline long		getFrameCount() const;

protected:
	// Ctor is passed the format and chans of frameBuffer.
	AudioDeviceImpl();
	virtual ~AudioDeviceImpl();
	// For subclass implementation.
	virtual int 	doOpen(int mode) = 0;
	virtual int 	doClose() = 0;
	virtual int 	doStart() = 0;
	virtual int 	doPause(bool) = 0;
	virtual int 	doStop() = 0;
	// Sets device format using settings as hints.
	virtual int 	doSetFormat(int sampfmt, int chans, double srate) = 0;
	// Returns actual write size and count via pWriteSize and pCount.
	virtual int 	doSetQueueSize(int *pWriteSize, int *pCount) = 0;
	virtual int 	doGetFrameCount() const = 0;
	// Returns number of frames read, or -1 for error.
	virtual	int		doGetFrames(void *frameBuffer, int frameCount) = 0;
	// Returns number of frames written, or -1 for error.
	virtual	int		doSendFrames(void *frameBuffer, int frameCount) = 0;
	// Local utilities for base classes to use.
	inline void		setDeviceParams(int fmt, int chans, double srate);
	inline bool		isRecording() const;
	inline bool		isPlaying() const;
	inline bool		isPassive() const;	// False if we don't run our own thread.
	int				getDeviceBytesPerFrame() const;
	inline void		setMode(int m);
	inline void		setState(State s);
	inline int		getMode() const;
	inline State	getState() const;
	inline Callback	getRunCallback() const;
	inline void *	getRunCallbackContext() const;
	inline Callback	getStopCallback() const;
	inline void *	getStopCallbackContext() const;
	void			*convertFrame(void *inFrame, void *outFrame, int frames, bool rec);
	int				error(const char *msg, const char *msg2=0);

private:
	void 			*createInterleavedBuffer(int fmt, int chans, int len);
	void 			destroyInterleavedBuffer(int fmt);
	void 			*createNoninterleavedBuffer(int fmt, int chans, int len);
	void 			destroyNoninterleavedBuffer(int fmt, int chans);
	int				createConvertBuffer(int frames);
	void			destroyConvertBuffer();
	int				setConvertFunctions(int rawSrcFormat, int rawDstFormat);

private:
	int					_mode;		// Playback, Record, etc.
	State 				_state;		// Open, Configured, etc.
	int					_frameFormat;
	int					_deviceFormat;
	int					_frameChannels;
	int					_deviceChannels;
	double				_samplingRate;
	Callback			_runCallback, _stopCallback;
	void				*_runCallbackContext, *_stopCallbackContext;
	void				*_convertBuffer;
	void				(*_recConvertFunction)(void *, void *, int, int);
	void				(*_playConvertFunction)(void *, void *, int, int);
	enum { ErrLength = 128 };
	char				_lastErr[ErrLength];
};

inline void	AudioDeviceImpl::setMode(int m) { _mode = m; }

inline void	AudioDeviceImpl::setState(State s) { _state = s; }

inline int AudioDeviceImpl::getMode() const { return _mode; }

inline bool	AudioDeviceImpl::isPassive() const { return (_mode & Passive) != 0; }

inline AudioDevice::State AudioDeviceImpl::getState() const { return _state; }

inline bool AudioDeviceImpl::isOpen() const { return _state >= Open; }

inline bool AudioDeviceImpl::isRunning() const { return _state >= Running; }

inline bool AudioDeviceImpl::isPaused() const { return _state == Paused; }

inline bool	AudioDeviceImpl::isRecording() const { return (_mode & Record) != 0; }

inline bool AudioDeviceImpl::isPlaying() const { return (_mode & Playback) != 0; }

inline int AudioDeviceImpl::getFrameFormat() const
{
	return MUS_GET_FORMAT(_frameFormat);
}

inline int AudioDeviceImpl::getDeviceFormat() const
{
	return MUS_GET_FORMAT(_deviceFormat);
}

inline bool AudioDeviceImpl::isFrameInterleaved() const
{
	return IS_INTERLEAVED_FORMAT(_frameFormat);
}

inline bool AudioDeviceImpl::isDeviceInterleaved() const
{
	return IS_INTERLEAVED_FORMAT(_deviceFormat);
}

inline bool AudioDeviceImpl::isFrameFmtNormalized() const
{
	return IS_NORMALIZED_FORMAT(_frameFormat);
}

inline bool	AudioDeviceImpl::isDeviceFmtNormalized() const
{
	return IS_NORMALIZED_FORMAT(_deviceFormat);
}

inline int AudioDeviceImpl::getFrameChannels() const
{
	return isOpen() ? _frameChannels : 0;
}

inline int AudioDeviceImpl::getDeviceChannels() const
{
	return  _deviceChannels;
}

inline double AudioDeviceImpl::getSamplingRate() const
{
	return _samplingRate;
}

inline long AudioDeviceImpl::getFrameCount() const
{
	return isOpen() ? doGetFrameCount() : 0;
}

inline void AudioDeviceImpl::setDeviceParams(int fmt, int chans, double rate)
{
	_deviceFormat = fmt;
	_deviceChannels = chans;
	_samplingRate = rate;
}


inline AudioDevice::Callback 
AudioDeviceImpl::getRunCallback() const { return _runCallback; }

inline void *
AudioDeviceImpl::getRunCallbackContext() const { return _runCallbackContext; }

inline AudioDevice::Callback 
AudioDeviceImpl::getStopCallback() const { return _stopCallback; }

inline void *
AudioDeviceImpl::getStopCallbackContext() const { return _stopCallbackContext; }

#endif	// _RT_AUDIODEVICEIMPL_H_
