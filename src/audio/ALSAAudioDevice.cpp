// ALSAAudioDevice.cpp

#ifdef ALSA

#include "ALSAAudioDevice.h"
#include <sys/time.h>
#include <sys/resource.h>	// setpriority()
#include <unistd.h>
#include <stdio.h>

#include <sndlibsupport.h>	// RTcmix header

inline int getstatus(int x) { return (x == 0) ? 0 : -1; }

ALSAAudioDevice::ALSAAudioDevice(const char *devName)
	: _deviceName(devName), _handle(NULL), _hwParams(NULL), _bufSize(0),
	  _stopDuringPause(false)
{
}

ALSAAudioDevice::~ALSAAudioDevice()
{
	close();
	snd_pcm_hw_params_free (_hwParams);
}

int ALSAAudioDevice::doOpen(int mode)
{
	int status;
	if ((mode & RecordPlayback) == RecordPlayback)
		return error("ALSA device does not yet support full duplex.");
	else if (mode & Record) {
		if ((status = snd_pcm_open(&_handle, 
								   _deviceName, 
								   SND_PCM_STREAM_CAPTURE, 0)) < 0)
		{
			return error("Cannot open audio input device: ", snd_strerror(status));
		}
	}
	else if (mode & Playback) {
		if ((status = snd_pcm_open(&_handle, 
								   _deviceName, 
								   SND_PCM_STREAM_PLAYBACK, 0)) < 0)
		{
			return error("Cannot open audio output device: ", snd_strerror(status));
		}
	}
	// Allocate hw params struct.
	if ((status = snd_pcm_hw_params_malloc(&_hwParams)) < 0) {
		error("Cannot allocate hardware parameter structure: ", 
			  snd_strerror (status));
	}
	if ((status = snd_pcm_hw_params_any(_handle, _hwParams)) < 0) {
		return error("Cannot initialize hardware parameter structure: ",
					 snd_strerror (status));
	}
	// HACK:  Get descriptor from pollfd's to use for select
	int count = snd_pcm_poll_descriptors_count(_handle);
	struct pollfd *pfds = new pollfd[count];
	snd_pcm_poll_descriptors(_handle, pfds, count);
	setDevice(pfds[0].fd);
	delete [] pfds;		
	return 0;
}

int ALSAAudioDevice::doClose()
{
	resetFrameCount();
//	printf("ALSAAudioDevice::doClose\n");
	setDevice(0);
	return snd_pcm_close(_handle);
}

static float zeroBuffer[8192];

int ALSAAudioDevice::doStart()
{
	int status;
	if ((status = snd_pcm_prepare(_handle)) < 0) {
		return error("Cannot prepare audio interface for use: ", 
					 snd_strerror (status));
	}
	if ((status = snd_pcm_wait(_handle, 500)) < 0) {
		return error("Error while waiting for device: ", snd_strerror (status));
	}
//	for (int buf=0; buf<8192; buf += _bufSize/4)
//		sendFrames(zeroBuffer, _bufSize/4);
	return ThreadedAudioDevice::startThread();
}

int ALSAAudioDevice::doPause(bool isPaused)
{
	int status = 0;
//	printf("ALSAAudioDevice::doPause(%d): stopping=%d paused=%d\n", paused, stopping(), _paused);
	status = snd_pcm_pause(_handle, isPaused);
	if (status == 0) {
		paused(isPaused);
	}
	return getstatus(status);
}

int ALSAAudioDevice::doStop()
{
	if (!stopping()) {
//		printf("ALSAAudioDevice::doStop: waiting for thread to finish...\n");
		stopping(true);		// signals play thread
		if (paused()) {
			_stopDuringPause = true;	// we handle this case as special
			snd_pcm_drop(_handle);
		}
		waitForThread();
	}
	return 0;
}

static snd_pcm_format_t convertSampleFormat(int sampfmt)
{
	snd_pcm_format_t sampleFormat = SND_PCM_FORMAT_UNKNOWN;
	switch (sampfmt) {
		case MUS_UBYTE:
			sampleFormat = SND_PCM_FORMAT_U8;
			break;
		case MUS_BYTE:
			sampleFormat = SND_PCM_FORMAT_S8;
			break;
		case MUS_LSHORT:
//			sampleFormat = SND_PCM_FORMAT_S16_LE;
//			break;
		case MUS_BSHORT:
			sampleFormat = SND_PCM_FORMAT_S16;
			break;
		case MUS_L24INT:
//			sampleFormat = SND_PCM_FORMAT_S24_LE;
//			break;
		case MUS_B24INT:
			sampleFormat = SND_PCM_FORMAT_S24;
			break;
		case MUS_LINT:
//			sampleFormat = SND_PCM_FORMAT_S32_LE;
//			break;
		case MUS_BINT:
			sampleFormat = SND_PCM_FORMAT_S32;
			break;
		case MUS_LFLOAT:
//			sampleFormat = SND_PCM_FORMAT_FLOAT_LE;
//			break;
		case MUS_BFLOAT:
			sampleFormat = SND_PCM_FORMAT_FLOAT;
			break;
		default:
			break;
	};
	return sampleFormat;
}

int ALSAAudioDevice::doSetFormat(int sampfmt, int chans, double srate)
{
	int status;
	printf("ALSAAudioDevice::doSetFormat: fmt: %d chans: %d rate: %g\n",
		   sampfmt, chans, srate);
	int deviceFormat = MUS_GET_FORMAT(sampfmt);
	snd_pcm_format_t sampleFormat = ::convertSampleFormat(deviceFormat);
	if (sampleFormat == SND_PCM_FORMAT_UNKNOWN)
		return error("Unknown or unsupported sample format");
	deviceFormat = NATIVE_FLOAT_FMT;	// This is what we report back.
	// Find a suitable format, ratcheting down from float to short.
	while ((status = snd_pcm_hw_params_set_format(_handle,
												  _hwParams, 
												  sampleFormat)) < 0)
	{
		if (sampleFormat == SND_PCM_FORMAT_FLOAT) {
			printf("\tTried float -- switching to 24bit.\n");
			sampleFormat = SND_PCM_FORMAT_S24;
			deviceFormat = MUS_L24INT;	// This is what we report back.
		}
		else if (sampleFormat == SND_PCM_FORMAT_S24) {
			printf("\tTried 24bit -- switching to 16bit.\n");
			sampleFormat = SND_PCM_FORMAT_S16;
			deviceFormat = NATIVE_SHORT_FMT;	// This is what we report back.
		}
		else
			break;	// give up here.
	}
	if (status < 0)
		return error("Cannot set sample format: ", snd_strerror(status));

	// set the subformat
	if ((status = snd_pcm_hw_params_set_subformat(_handle, _hwParams,
										  		  SND_PCM_SUBFORMAT_STD)) < 0) {
		return error("Cannot set sample subformat: ", snd_strerror(status));
	}
	unsigned reqRate = (unsigned) (srate + 0.5);
	if ((status = snd_pcm_hw_params_set_rate_near(_handle, 
												  _hwParams, 
												  &reqRate, 0)) < 0) {
		return error("cannot set sample rate: ", snd_strerror(status));
	}
	
	if (reqRate != (unsigned) (srate + 0.5)) {
		char msg[64];
		sprintf(msg, "Cannot set rate to %g (got %u)", srate, reqRate);
		return error(msg);
	}
	
	if ((status = snd_pcm_hw_params_set_channels(_handle,
												 _hwParams, 
												 chans)) < 0) {
		return error("Cannot set channel count: ", snd_strerror(status));
	}
	
	// Try setting interleave to match what we will be handed.
	snd_pcm_access_t hwAccess = isFrameInterleaved() ? 
			SND_PCM_ACCESS_RW_INTERLEAVED : SND_PCM_ACCESS_RW_NONINTERLEAVED;

	while ((status = snd_pcm_hw_params_set_access(_handle,
												  _hwParams,
												  hwAccess)) < 0)
	{
		// Couldn't do it.  Flip interleave.
		if (hwAccess == SND_PCM_ACCESS_RW_INTERLEAVED)
			hwAccess = SND_PCM_ACCESS_RW_NONINTERLEAVED;
		else if (hwAccess == SND_PCM_ACCESS_RW_NONINTERLEAVED)
			hwAccess = SND_PCM_ACCESS_RW_INTERLEAVED;
		else
			break;	// give up.
	}
	if (status < 0)
		return error("Cannot set access type: ", snd_strerror(status));

	// Store the device params to allow format conversion.
	if (hwAccess == SND_PCM_ACCESS_RW_INTERLEAVED) {
		printf("\tHW uses interleaved channels\n");
		deviceFormat |= MUS_INTERLEAVED;
	}
	else {
		printf("\tHW uses non-interleaved channels\n");
		deviceFormat |= MUS_NON_INTERLEAVED;
	}

	setDeviceParams(deviceFormat, chans, srate);

	return 0;
}

int ALSAAudioDevice::doSetQueueSize(int *pWriteSize, int *pCount)
{
	_bufSize = (snd_pcm_uframes_t) *pWriteSize * *pCount;
	int status;
 	if ((status = snd_pcm_hw_params_set_buffer_size_near(_handle,
														 _hwParams,
														 &_bufSize)) < 0)
	{
 		return error("Cannot set buffer size: ", snd_strerror(status));
 	}
	printf("ALSAAudioDevice::doSetQueueSize: requested %d frames total, got %d\n",
			*pWriteSize * *pCount, (int) _bufSize);
	
	int dir = 0;
	snd_pcm_uframes_t periodsize = *pWriteSize, tryperiodsize;

	tryperiodsize = periodsize;
	if ((status = snd_pcm_hw_params_set_period_size_near(_handle,
														 _hwParams,
														 &tryperiodsize,
														 &dir)) < 0)
	{
		return error("Failed to set ALSA period size: ", snd_strerror(status));
	}
	else
		printf("ALSAAudioDevice::doSetQueueSize: requested period size near %d, got %d\n",
			   (int) periodsize, (int)tryperiodsize);

	unsigned periods = *pCount;

	printf("setting periods to %d\n", periods);
	if ((status = snd_pcm_hw_params_set_periods(_handle,
												_hwParams, 
												periods, 
												dir)) < 0)
	{
		return error("Failed to set ALSA periods: ", snd_strerror(status));
	}

 	if ((status = snd_pcm_hw_params_get_buffer_size(_hwParams, &_bufSize)) < 0) {
		return error("Cannot retrieve buffer size: ", snd_strerror(status));
	}

	if ((status = snd_pcm_hw_params (_handle, _hwParams)) < 0) {
		return error("Cannot set parameters: ", snd_strerror(status));
	}

	// Set up device to wake us whenever *pWriteSize frames can be read/written
	
	snd_pcm_sw_params_t *swParams;
	if ((status = snd_pcm_sw_params_malloc(&swParams)) < 0) {
		return error("Cannot allocate sw params structure: ", 
					 snd_strerror (status));
	}
	if ((status = snd_pcm_sw_params_current(_handle, swParams)) < 0) {
		return error("Cannot initialize sw params: ", snd_strerror (status));
	}
	printf("Testing: hw will wake us when %d frames can be read/written\n", *pWriteSize);
	if ((status = snd_pcm_sw_params_set_avail_min(_handle, swParams, *pWriteSize)) < 0) {
		return error("Cannot set minimum available count: ", 
					 snd_strerror (status));
	}
	if ((status = snd_pcm_sw_params_set_start_threshold(_handle, swParams, 0L)) < 0) {
		return error("Cannot set start mode: ", snd_strerror (status));
	}
	if ((status = snd_pcm_sw_params(_handle, swParams)) < 0) {
		return error("Cannot set software params: ", snd_strerror (status));
	}
	snd_pcm_sw_params_free(swParams);
	
	*pWriteSize = tryperiodsize;
	return getstatus(status);
}

int	ALSAAudioDevice::doGetFrames(void *frameBuffer, int frameCount)
{
	int fread = -1;
	while (fread < 0) {
		if (isDeviceInterleaved())
			fread = snd_pcm_readi(_handle, frameBuffer, frameCount);
		else
			fread = snd_pcm_readn(_handle, (void **) frameBuffer, frameCount);
		if (fread == -EPIPE) {
			printf("ALSAAudioDevice::doGetFrames: overrun on read -- recovering and calling again\n");
			snd_pcm_prepare(_handle);
		}
		else if (fread < 0) {
			fprintf(stderr, "ALSAAudioDevice::doGetFrames: error reading from device: %s\n", snd_strerror(fread));
			return error("Error reading from device: ", snd_strerror(fread));
		}
	}
	
	if (fread > 0) {
		if (fread < frameCount)
			printf("ALSAAudioDevice::doGetFrames: read %d out of %d frames!\n",
				   fread, frameCount);
		incrementFrameCount(fread);
		return fread;
	}
	return -1;
}

int	ALSAAudioDevice::doSendFrames(void *frameBuffer, int frameCount)
{
	int fwritten = -1;
	while (fwritten < 0) {
		if (isDeviceInterleaved())
			fwritten = snd_pcm_writei(_handle, frameBuffer, frameCount);
		else
			fwritten = snd_pcm_writen(_handle, (void **) frameBuffer, frameCount);
//		printf("ALSAAudioDevice::doSendFrames: %d frames to write, %d written\n", frameCount, fwritten);
		if (fwritten == -EPIPE) {
//			printf("ALSAAudioDevice::doSendFrames: underrun on write -- recovering and calling again\n");
			snd_pcm_prepare(_handle);
		}
		else if (fwritten < 0) {
			fprintf(stderr, "ALSAAudioDevice::doSendFrames: error writing to device: %s\n", snd_strerror(fwritten));
			return error("Error writing to device: ", snd_strerror(fwritten));
		}
	}
	if (fwritten > 0) {
		if (fwritten < frameCount)
			printf("ALSAAudioDevice::doSendFrames: write %d out of %d frames!\n",
				   fwritten, frameCount);
		incrementFrameCount(fwritten);
		return fwritten;
	}
	return -1;
}

void ALSAAudioDevice::run()
{
	Callback runCallback = getRunCallback();
//	printf("ALSAAudioDevice::run: top of loop\n");
	while (waitForDevice(0) == true) {
		if ((*runCallback)(this, getRunCallbackContext()) != true) {
//			printf("ALSAAudioDevice::run: callback returned false\n");
			break;
		}
	}
//	printf("ALSAAudioDevice::run: after loop\n");
	if (!_stopDuringPause) {
		snd_pcm_drain(_handle);
		_stopDuringPause = false;	// reset
	}
	setState(Configured);	// no longer running
	Callback stopCallback = getStopCallback();
	if (stopCallback) {
		(*stopCallback)(this, getStopCallbackContext());
	}
//	printf("ALSAAudioDevice::run: thread exiting\n");
}

AudioDevice *createAudioDevice(const char *path)
{
	return new ALSAAudioDevice(path);
}

#endif	// ALSA
