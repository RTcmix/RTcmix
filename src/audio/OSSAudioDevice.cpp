// OSSAudioDevice.cpp

#if defined(LINUX)

#include <math.h>
#include "OSSAudioDevice.h"
#include "AudioIODevice.h"
#include <sys/soundcard.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>	// strerror()

#define DEBUG 0

#if DEBUG > 1
#define PRINT0 if (1) printf
#define PRINT1 if (1) printf
#elif DEBUG > 0
#define PRINT0 if (1) printf
#define PRINT1 if (0) printf
#else
#define PRINT0 if (0) printf
#define PRINT1 if (0) printf
#endif

OSSAudioDevice::OSSAudioDevice(const char *devPath)
	: _inputDeviceName(NULL), _outputDeviceName(NULL), _bytesPerFrame(0)
{
	_inputDeviceName = new char [strlen(devPath) + 1];
	_outputDeviceName = new char [strlen(devPath) + 1];
	strcpy(_inputDeviceName, devPath);
	strcpy(_outputDeviceName, devPath);
}

OSSAudioDevice::~OSSAudioDevice()
{
	close();
	delete [] _inputDeviceName;
	delete [] _outputDeviceName;
}

int OSSAudioDevice::doClose()
{
	int status = 0;
	if (!closing()) {
		closing(true);
		resetFrameCount();
		_bufferSize = 0;	// lets us know it must be recalculated.
		PRINT0("\tOSSAudioDevice::doClose\n");
		status = closeDevice();
	}
	return status;
}

int OSSAudioDevice::doStart()
{
	// Get this now if user never set it.
	if (_bufferSize == 0) {
		if (ioctl(SNDCTL_DSP_GETBLKSIZE, &_bufferSize) == -1) {
			return error("Error while retrieving block size.");
		}
	}
	return ThreadedAudioDevice::startThread();
}

int OSSAudioDevice::doPause(bool isPaused)
{
	paused(isPaused);
	return 0;
}


int
OSSAudioDevice::setDeviceFormat(int dev, int sampleFormat, int chans, int srate)
{
	int confirmedFormat = sampleFormat;
	if (::ioctl(dev, SNDCTL_DSP_SETFMT, &confirmedFormat))
		return error("OSS error while setting sample format: ", strerror(errno));
	else if (confirmedFormat != sampleFormat)
		return error("This sample format not supported by device.");
#ifndef SOUND_PCM_WRITE_CHANNELS	// OLD VERSION
	if (chans != 1 && chans != 2)
		return error("This device supports only mono and stereo");
#else
	int reqChans = chans;
	if (::ioctl(dev, SOUND_PCM_WRITE_CHANNELS, &reqChans) || reqChans != chans)
#endif
	{
		int dsp_stereo = (chans == 2);
		if (::ioctl(dev, SNDCTL_DSP_STEREO, &dsp_stereo) || dsp_stereo != (chans == 2))
			return error("OSS error while setting channel count: ", strerror(errno));
	}
	int dsp_speed = (int) srate;
	if (::ioctl(dev, SNDCTL_DSP_SPEED, &dsp_speed))
		return error("OSS error while setting sample rate: ", strerror(errno));
	if (dsp_speed != (int) srate)
		return error("Device does not support this sample rate");
#ifdef SOUND_PCM_WRITE_CHANNELS
	PRINT0("OSSAudioDevice::setDeviceFormat: srate = %d, channels = %d\n", dsp_speed, reqChans);
#endif
	return 0;
}

int
OSSAudioDevice::ioctl(int req, void *argp) {
	return ::ioctl(device(), req, argp);
}

static char zeroBuffer[32768];

void OSSAudioDevice::run()
{
	audio_buf_info info;
	PRINT1("OSSAudioDevice::run: top of loop\n");
	while (waitForDevice(0) == true) {
		if (ioctl(isPlaying() ? SNDCTL_DSP_GETOSPACE : SNDCTL_DSP_GETISPACE,
				  &info))
		{
			error("OSS error: ", strerror(errno));
			break;
		}
		if (info.bytes < bufferSize() / 2) {
			PRINT1("\tOSSAudioDevice::run: %d bytes avail...waiting\n", info.bytes);
			usleep(10);
			continue;
		}
		if (runCallback() != true) {
			break;
		}
		// Spin if device is paused
		if (paused()) {
			ioctl(SNDCTL_DSP_POST, 0);
			while (paused()) {
				usleep(1000);
			}
		}
	}
	// Write buffer of zeros.
	doSendFrames(zeroBuffer, sizeof(zeroBuffer)/getDeviceBytesPerFrame());
	
	PRINT1("OSSAudioDevice::run: flushing...\n");
	// Flush device.
	ioctl(SNDCTL_DSP_SYNC, 0);

	// If we stopped due to callback being done, set the state so that the
	// call to close() does not attempt to call stop, which we cannot do in
	// this thread.  Then, check to see if we are being closed by the main
	// thread before calling close() here, to avoid a reentrant call.
	
	if (!stopping()) {
		setState(Configured);
		if (!closing()) {
			PRINT1("OSSAudioDevice::run: calling close()\n");
			close();
		}
	}
//		printf("OSSAudioDevice::run: stop callback\n");
	stopCallback();
//	printf("OSSAudioDevice::run: thread exiting\n");
}

#endif	// LINUX

