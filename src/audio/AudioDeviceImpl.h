// AudioDeviceImpl.h -- partial implementation for basic devices

#ifndef _RT_AUDIODEVICEIMPL_H_
#define _RT_AUDIODEVICEIMPL_H_

#include "AudioDevice.h"

class AudioDeviceImpl : public AudioDevice {
public:
	// Redefined from AudioDevice
	int			open(int mode, int sampfmt, int chans, double srate);
	int			close();
	int			start(Callback runCallback, void *callbackContext);
	int			setStopCallback(Callback stopCallback, void *callbackContext);
	int			pause(bool);
	int			stop();
	int			setFormat(int sampfmt, int chans, double srate);
	int			setQueueSize(int *pFrames);
	int			getFrames(void *frameBuffer, int frameCount);
	int			sendFrames(void *frameBuffer, int frameCount);
	bool		isOpen() const;
	bool		isRunning() const;
	bool		isPaused() const;
	int			getFormat() const;
	int			getChannels() const;
	double		getSamplingRate() const;
	long		getFrameCount() const;
	const char *getLastError() const;
protected:
	AudioDeviceImpl();
	// For subclass implementation.
	virtual int doOpen(int mode) = 0;
	virtual int doClose() = 0;
	virtual int doStart() = 0;
	virtual int doPause(bool) = 0;
	virtual int doStop() = 0;
	virtual int doSetFormat(int sampfmt, int chans, double srate) = 0;
	// Returns actual queue size set via pFrameSize.
	virtual int doSetQueueSize(int *pQueueSize) = 0;
	virtual int doGetFrameCount() const = 0;
	// Returns number of frames read, or -1 for error.
	virtual	int	doGetFrames(void *frameBuffer, int frameCount) = 0;
	// Returns number of frames written, or -1 for error.
	virtual	int	doSendFrames(void *frameBuffer, int frameCount) = 0;
	// Local utilities for base classes to use.
	inline bool		isRecording() const;
	inline bool		isPlaying() const;
	inline bool		isPassive() const;	// False if we don't run our own thread.
	int				getBytesPerFrame() const;
	inline void		setMode(int m);
	inline void		setState(State s);
	inline int		getMode() const;
	inline State	getState() const;
	inline Callback	getRunCallback() const;
	inline void *	getRunCallbackContext() const;
	inline Callback	getStopCallback() const;
	inline void *	getStopCallbackContext() const;
	int				error(const char *msg, const char *msg2=0);
private:
	int			_mode;		// Playback, Record, etc.
	State 		_state;		// Open, Configured, etc.
	int			_sampFormat;
	int			_channels;
	double		_samplingRate;
	Callback	_runCallback, _stopCallback;
	void		*_runCallbackContext, *_stopCallbackContext;
	enum { ErrLength = 128 };
	char	_lastErr[ErrLength];
};

inline void	AudioDeviceImpl::setMode(int m) { _mode = m; }

inline void	AudioDeviceImpl::setState(State s) { _state = s; }

inline int AudioDeviceImpl::getMode() const { return _mode; }

inline bool	AudioDeviceImpl::isPassive() const { return (_mode & Passive) != 0; }

inline AudioDevice::State AudioDeviceImpl::getState() const { return _state; }

inline bool AudioDeviceImpl::isOpen() const { return _state >= Open; }

inline bool AudioDeviceImpl::isRunning() const { return _state >= Running; }

inline bool AudioDeviceImpl::isPaused() const { return _state == Paused; }

inline bool	AudioDeviceImpl::isRecording() const { return isRunning() && (_mode & Record); }

inline bool AudioDeviceImpl::isPlaying() const { return isRunning() && (_mode & Playback); }

inline AudioDevice::Callback 
AudioDeviceImpl::getRunCallback() const { return _runCallback; }

inline void *
AudioDeviceImpl::getRunCallbackContext() const { return _runCallbackContext; }

inline AudioDevice::Callback 
AudioDeviceImpl::getStopCallback() const { return _stopCallback; }

inline void *
AudioDeviceImpl::getStopCallbackContext() const { return _stopCallbackContext; }

#endif	// _RT_AUDIODEVICEIMPL_H_
