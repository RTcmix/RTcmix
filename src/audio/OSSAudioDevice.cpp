// OSSAudioDevice.cpp

#if defined(LINUX)

#ifndef ALSA

#include "OSSAudioDevice.h"
#include <sys/soundcard.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>	// strerror()

#include <sndlibsupport.h>	// RTcmix header

OSSAudioDevice::OSSAudioDevice(const char *devPath)
	: _inputDeviceName(devPath), _outputDeviceName(devPath), _bytesPerFrame(0)
{
}

OSSAudioDevice::~OSSAudioDevice()
{
	close();
}

int OSSAudioDevice::doOpen(int mode)
{
	int fd = 0;
	switch (mode & DirectionMask) {
	case Playback:
		setDevice(fd = ::open(_outputDeviceName, O_WRONLY));
		closing(false);
		break;
	case Record:
		setDevice(fd = ::open(_inputDeviceName, O_RDONLY));
		closing(false);
		break;
	case RecordPlayback:
		setDevice(fd = ::open(_outputDeviceName, O_RDWR));
		closing(false);
		break;
	default:
		error("AudioDevice: Illegal open mode.");
	}
	return (fd > 0) ? 0 : -1;
}

int OSSAudioDevice::doClose()
{
	int status = 0;
	if (!closing()) {
		closing(true);
		resetFrameCount();
		_bufferSize = 0;	// lets us know it must be recalculated.
//		printf("\tOSSAudioDevice::doClose\n");
		status = ::close(device());
		setDevice(0);
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


int OSSAudioDevice::doSetFormat(int sampfmt, int chans, double srate)
{
	int sampleFormat;
	int deviceFormat = MUS_GET_FORMAT(sampfmt);
	switch (MUS_GET_FORMAT(sampfmt)) {
		case MUS_UBYTE:
			sampleFormat = AFMT_U8;
			_bytesPerFrame = chans;
			break;
		case MUS_BYTE:
			sampleFormat = AFMT_S8;
			_bytesPerFrame = chans;
			break;
		case MUS_LFLOAT:
			deviceFormat = NATIVE_SHORT_FMT;
		case MUS_LSHORT:
			_bytesPerFrame = 2 * chans;
			sampleFormat = AFMT_S16_LE;
			break;
		case MUS_BFLOAT:
			deviceFormat = NATIVE_SHORT_FMT;
		case MUS_BSHORT:
			_bytesPerFrame = 2 * chans;
			sampleFormat = AFMT_S16_BE;
			break;
		default:
			_bytesPerFrame = 0;
			return error("Unsupported sample format");
	};
	int confirmedFormat = sampleFormat;
	if (ioctl(SNDCTL_DSP_SETFMT, &confirmedFormat))
		return error("Error while setting sample format.");
	else if (confirmedFormat != sampleFormat)
		return error("This sample format not supported by device.");
#ifndef SOUND_PCM_WRITE_CHANNELS	// OLD VERSION
	if (chans != 1 && chans != 2)
		return error("This device supports only mono and stereo");
#else
	int reqChans = chans;
	if (ioctl(SOUND_PCM_WRITE_CHANNELS, &reqChans) || reqChans != chans)
#endif
	{
		int dsp_stereo = (chans == 2);
		if (ioctl(SNDCTL_DSP_STEREO, &dsp_stereo) || dsp_stereo != (chans == 2))
			return error("Unable to set channel count.");
	}
	int dsp_speed = (int) srate;
	if (ioctl (SNDCTL_DSP_SPEED, &dsp_speed))
		return error("Error while setting sample rate.");
	if (dsp_speed != (int) srate)
		return error("Device does not support this sample rate");
	// Store the device params to allow format conversion.
	setDeviceParams(deviceFormat | MUS_INTERLEAVED,		// always interleaved
					chans,
					srate);
	return 0;
}

int OSSAudioDevice::doSetQueueSize(int *pWriteSize, int *pCount)
{
	int reqQueueBytes = *pWriteSize * getDeviceBytesPerFrame();
	int queuecode = ((int) (log(reqQueueBytes) / log(2.0))) + 1;
	int sizeCode = (*pCount << 16) | (queuecode & 0x0000ffff);
	if (ioctl(SNDCTL_DSP_SETFRAGMENT, &sizeCode) == -1) {
		printf("ioctl(SNDCTL_DSP_SETFRAGMENT, ...) returned -1\n");
	}
	_bufferSize = 0;
	if (ioctl(SNDCTL_DSP_GETBLKSIZE, &_bufferSize) == -1) {
		return error("Error while retrieving block size.");
	}
	*pWriteSize = _bufferSize / (*pCount * getDeviceBytesPerFrame());
	return 0;
}

int	OSSAudioDevice::doGetFrames(void *frameBuffer, int frameCount)
{
	int toRead = frameCount * _bytesPerFrame;
	int read = ::read(device(), frameBuffer, toRead);
//	printf("OSSAudioDevice::doGetFrames: %d bytes to read, %d read\n", toRead, read);
	if (read > 0) {
		int frames = read / _bytesPerFrame;
		incrementFrameCount(frames);
		return frames;
	}
	else {
		return error("Error reading from device:", strerror(read));
	}
}

int	OSSAudioDevice::doSendFrames(void *frameBuffer, int frameCount)
{
	int toWrite = frameCount * _bytesPerFrame;
	int written = ::write(device(), frameBuffer, toWrite);
//	printf("OSSAudioDevice::doSendFrames: %d bytes to write, %d written\n", toWrite, written);
	if (written > 0) {
		int frames = written / _bytesPerFrame;
		incrementFrameCount(frames);
		return frames;
	}
	else {
		return error("Error writing to device.");
	}
}

int
OSSAudioDevice::ioctl(int req, void *argp) {
	return ::ioctl(device(), req, argp);
}

void OSSAudioDevice::run()
{
	audio_buf_info info;
	Callback runCallback = getRunCallback();
//	printf("OSSAudioDevice::run: top of loop\n");
	while (waitForDevice(0) == true) {
		if (ioctl(SNDCTL_DSP_GETOSPACE, &info)) {
			error("Error during playback");
			return;
		}
//		printf("OSSAudioDevice::run: %d bytes avail\n", info.bytes);
		if (info.bytes < bufferSize() / 2)
			continue;
		if ((*runCallback)(this, getRunCallbackContext()) != true) {
//			printf("OSSAudioDevice::run: callback returned false\n");
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
//	static int buffer[256];
//	::write(device(), buffer, sizeof(buffer));
//	printf("OSSAudioDevice::run: flushing...\n");
	// Flush device.
	ioctl(SNDCTL_DSP_SYNC, 0);

	// If we stopped due to callback being done, set the state so that the
	// call to close() does not attempt to call stop, which we cannot do in
	// this thread.  Then, check to see if we are being closed by the main
	// thread before calling close() here, to avoid a reentrant call.
	
	if (!stopping()) {
		setState(Configured);
		if (!closing()) {
//			printf("OSSAudioDevice::run: calling close()\n");
			close();
		}
	}
	Callback stopCallback = getStopCallback();
	if (stopCallback) {
//		printf("OSSAudioDevice::run: stop callback\n");
		(*stopCallback)(this, getStopCallbackContext());
	}
//	printf("OSSAudioDevice::run: thread exiting\n");
}

AudioDevice *createAudioDevice(const char *path)
{
	return new OSSAudioDevice(path);
}

#endif	// !ALSA

#endif	// LINUX

