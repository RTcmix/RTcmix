// AudioIODevice.cpp

#include "AudioIODevice.h"

 
AudioIODevice::AudioIODevice(AudioDevice *inputDevice,
							 AudioDevice *outputDevice)
	: _inputDevice(inputDevice), _outputDevice(outputDevice)
{
}

AudioIODevice::~AudioIODevice()
{
	delete _outputDevice;
	delete _inputDevice;
}

int AudioIODevice::setFrameFormat(int sampfmt, int chans)
{
	int status = _inputDevice->setFrameFormat(sampfmt, chans);
	if (status == 0)
		status = _outputDevice->setFrameFormat(sampfmt, chans);
	return status;
}

int AudioIODevice::open(int mode, int sampfmt, int chans, double sr)
{
	if ((mode & DirectionMask) != RecordPlayback)
		return -1;
	
	int status = _inputDevice->open(Record | Passive, sampfmt, chans, sr);
	if (status == 0)
		status = _outputDevice->open(Playback, sampfmt, chans, sr);
	return status;
}

int AudioIODevice::close()
{
	int status = _outputDevice->close();
	if (status == 0)
		status = _inputDevice->close();
	return status;
}

int AudioIODevice::start(Callback runCallback, void *callbackContext)
{
	// Passing 0 here because input device is opened passively.
	int status = _inputDevice->start(0, 0);
	if (status == 0)
		status = _outputDevice->start(runCallback, callbackContext);
	return status;
}

int AudioIODevice::setStopCallback(Callback stopCallback, void *callbackContext)
{
	int status = _outputDevice->setStopCallback(stopCallback, callbackContext);
	return status;
}

int AudioIODevice::pause(bool paused)
{
	int status = _outputDevice->pause(paused);
	return status;
}

int AudioIODevice::stop()
{
	int status = _outputDevice->stop();
	if (status == 0)
		status = _inputDevice->stop();
	return status;
}

int AudioIODevice::setFormat(int fmt, int chans, double srate)
{
	int status = _outputDevice->setFormat(fmt, chans, srate);
	if (status == 0)
		status = _inputDevice->setFormat(fmt, chans, srate);
	return status;
}

int AudioIODevice::setQueueSize(int *pWriteSize, int *pCount)
{
	int status = _outputDevice->setQueueSize(pWriteSize, pCount);
	if (status == 0)
		status = _inputDevice->setQueueSize(pWriteSize, pCount);
	return status;
}

int AudioIODevice::getFrames(void *frameBuffer, int frameCount)
{
	int status = _inputDevice->getFrames(frameBuffer, frameCount);
	return status;
}

int AudioIODevice::sendFrames(void *frameBuffer, int frameCount)
{
	int status = _outputDevice->getFrames(frameBuffer, frameCount);
	return status;
}

bool AudioIODevice::isOpen() const
{
	return _outputDevice->isOpen() && _inputDevice->isOpen();
}

bool AudioIODevice::isRunning() const
{
	return _outputDevice->isRunning();
}

bool AudioIODevice::isPaused() const
{
	return _outputDevice->isPaused();
}

const char *AudioIODevice::getLastError() const
{
	const char *theError = _outputDevice->getLastError();
	if (!theError)
		theError = _inputDevice->getLastError();
	return theError;
}
