// AudioIODevice.cpp

#include "AudioIODevice.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
 
// This subclass is designed to allow two independent AudioDevice instances
// to function and be controlled as a single unit.  This allows systems which
// do not support full duplex audio through a single device to create two
// half-duplex devices and then wrap them as a full-duplex one.  It also allows
// for the creation of compound AudioDevices which record from one HW device
// and play out to another.
//
// In order to support the AudioDevice thread-driven model, only one of the
// AudioDevices can run in Active mode (where it spawns a child thread and
// invokes the run() method).  The other must run in Passive mode, where it
// waits for the application to call getFrames() or sendFrames() on it.  The
// final argument to the constructor, below, sets whether the input device
// drives the system or whether than is done by the output device.  The class
// handles the opening of the devices to make this work correctly.

AudioIODevice::AudioIODevice(AudioDevice *inputDevice,
							 AudioDevice *outputDevice,
							 bool inputIsActive)
	: _inputDevice(inputDevice), _outputDevice(outputDevice),
	  _inputActive(inputIsActive)
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
	int inmode, outmode;
	if (!_inputActive) {
		inmode = Record | Passive;
		outmode = Playback;
	}
	else {
		inmode = Record;
		outmode = Playback | Passive;
	}
//	printf("AudioIODevice::open: opening input and output devices\n");
	int status = _inputDevice->open(inmode, sampfmt, chans, sr);
	if (status == 0)
		status = _outputDevice->open(outmode, sampfmt, chans, sr);
	return status;
}

int AudioIODevice::close()
{
	int status = getActiveDevice()->close();
	if (status == 0)
		status = getPassiveDevice()->close();
	return status;
}

int AudioIODevice::start(Callback *runCallback)
{
	int status = 0;
//	printf("AudioIODevice::start: starting input and output devices\n");
	if ((status = getPassiveDevice()->start(0, 0)) == 0)
		status = getActiveDevice()->start(runCallback);
	return status;
}

int AudioIODevice::setStopCallback(Callback *stopCallback)
{
	return getActiveDevice()->setStopCallback(stopCallback);
}

bool AudioIODevice::runCallback()
{
	assert(0);	// never supposed to be called!
	return false;
}

bool AudioIODevice::stopCallback() {
	assert(0);	// never supposed to be called!
	return false;
}

int AudioIODevice::pause(bool paused)
{
	int status = getActiveDevice()->pause(paused);
	return status;
}

int AudioIODevice::stop()
{
	int status = getActiveDevice()->stop();
	if (status == 0)
		status = getPassiveDevice()->stop();
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
	int status = _outputDevice->sendFrames(frameBuffer, frameCount);
	return status;
}

bool AudioIODevice::isOpen() const
{
	return _outputDevice->isOpen() && _inputDevice->isOpen();
}

bool AudioIODevice::isRunning() const
{
	return getActiveDevice()->isRunning();
}

bool AudioIODevice::isPaused() const
{
	return getActiveDevice()->isPaused();
}

const char *AudioIODevice::getLastError() const
{
	const char *theError = getActiveDevice()->getLastError();
	if (strlen(theError) == 0)
		theError = getPassiveDevice()->getLastError();
	return theError;
}
