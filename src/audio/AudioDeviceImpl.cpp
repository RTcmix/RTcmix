// AudioDeviceImpl.cpp

#include "AudioDeviceImpl.h"
#include <string.h>
#include <stdio.h>

#include <sndlibsupport.h>	// RTcmix header

AudioDeviceImpl::AudioDeviceImpl() 
	: _mode(Unset), _state(Closed), _sampFormat(MUS_UNSUPPORTED), _channels(0),
	  _samplingRate(0.0),
	  _runCallback(NULL), _stopCallback(NULL),
	  _runCallbackContext(NULL), _stopCallbackContext(NULL)
{
}

int AudioDeviceImpl::open(int mode, int sampfmt, int chans, double srate)
{
//	printf("AudioDeviceImpl::open\n");
	_lastErr[0] = 0;
	setMode(mode);
	int status = 0;
	close();
	if ((status = doOpen(mode)) == 0) {
		setState(Open);
		if ((status = setFormat(sampfmt, chans, srate)) != 0)
			close();
	}
	return status;
}

int AudioDeviceImpl::close()
{
//	printf("AudioDeviceImpl::close -- begin\n");
	int status = 0;
	if (isOpen()) {
//		printf("AudioDeviceImpl::close: was open, calling stop()\n");
		stop();
//		printf("AudioDeviceImpl::close: now calling doClose()\n");
		if ((status = doClose()) == 0) {
			setState(Closed);
//			printf("AudioDeviceImpl::close: state now set to Closed\n");
		}
	}
//	printf("AudioDeviceImpl::close -- finish\n");
	return status;
}

int AudioDeviceImpl::start(AudioDevice::Callback callback, void *context)
{
	int status = 0;
	if (!isRunning()) {
		_runCallback = callback;
		_runCallbackContext = context;
		State oldState = getState();
		setState(Running);
		if ((status = doStart()) != 0) {
			setState(oldState);
		}
	}
	return status;
}

int AudioDeviceImpl::pause(bool willPause)
{
	int status = 0;
	if (isRunning() && isPaused() != willPause) {
		if ((status = doPause(willPause)) == 0) {
			setState(willPause ? Paused : Running);
		}
	}
	return status;
}

int AudioDeviceImpl::setStopCallback(Callback stopCallback, void *callbackContext)
{
	_stopCallback = stopCallback;
	_stopCallbackContext = callbackContext;
	return 0;
}

int AudioDeviceImpl::stop()
{
//	printf("AudioDeviceImpl::stop -- begin\n");
	int status = 0;
	if (isRunning()) {
//		printf("AudioDeviceImpl::stop: was running, calling doStop()\n");
		if ((status = doStop()) == 0) {
			setState(Configured);
		}
	}
	_runCallback = NULL;
	_runCallbackContext = NULL;
//	printf("AudioDeviceImpl::stop -- finish\n");
	return status;
}

int AudioDeviceImpl::getFrames(void *frameBuffer, int frameCount)
{
	return isRecording() ? doGetFrames(frameBuffer, frameCount) : error("Not in record mode");
}

int AudioDeviceImpl::sendFrames(void *frameBuffer, int frameCount)
{
	return isPlaying() ? doSendFrames(frameBuffer, frameCount) : error("Not in playback mode");
}

const char *AudioDeviceImpl::getLastError() const {
	return _lastErr;
}

int AudioDeviceImpl::error(const char *msg, const char *msg2)
{
	sprintf(_lastErr, "AudioDevice: %s%s", msg, msg2 ? msg2 : "");
	return -1;
}

int AudioDeviceImpl::setFormat(int sampfmt, int chans, double srate)
{
	if (isOpen()) {
		if (doSetFormat(sampfmt, chans, srate) == 0) {
			_sampFormat = sampfmt;
			_channels = chans;
			_samplingRate = srate;
			setState(Configured);
			return 0;
		}
		else return -1;
	}
	return error("Audio device not open");
}

int AudioDeviceImpl::setQueueSize(int *pFrames)
{
	int reqQueueSize = *pFrames;
	switch (getState()) {
	case Configured:
		break;
	case Running:
	case Paused:
		if (isPassive())
			break;	// In passive mode we can do this anytime.
		// else
		*pFrames = -1;
		return error("Cannot set queue size while running");
	case Open:
		*pFrames = -1;
		return error("Cannot set queue size before setting format");
	case Closed:
		*pFrames = -1;
		return error("Audio device not open");
	default:
		*pFrames = -1;
		return error("Unknown state!");
	}
	
	if (doSetQueueSize(&reqQueueSize) == 0) {
		*pFrames = reqQueueSize;
		return 0;
	}
	*pFrames = -1;	// error condition
	return -1;
}

int AudioDeviceImpl::getBytesPerFrame() const
{
	return mus_data_format_to_bytes_per_sample(_sampFormat) * _channels;
}

int AudioDeviceImpl::getFormat() const
{
	return isOpen() ? _sampFormat : MUS_UNSUPPORTED;
}

int AudioDeviceImpl::getChannels() const
{
	return isOpen() ? _channels : 0;
}

double AudioDeviceImpl::getSamplingRate() const
{
	return isOpen() ? _samplingRate : 0.0;
}

long AudioDeviceImpl::getFrameCount() const
{
	return isOpen() ? doGetFrameCount() : 0;
}

