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
	int status;
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
	if ((status = snd_pcm_hw_params_set_access(_handle,
											   _hwParams,
											   SND_PCM_ACCESS_RW_INTERLEAVED))
   		 < 0)
	{
		return error("Cannot set access type: ", snd_strerror(status));
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
	printf("ALSAAudioDevice::doClose\n");
	setDevice(0);
	return snd_pcm_close(_handle);
}

static short zeroBuffer[8192];

int ALSAAudioDevice::doStart()
{
	int status;
	if ((status = snd_pcm_hw_params (_handle, _hwParams)) < 0) {
		return error("Cannot set parameters: ", snd_strerror(status));
	}
	if ((status = snd_pcm_prepare(_handle)) < 0) {
		return error("Cannot prepare audio interface for use: ", 
					 snd_strerror (status));
	}
	if ((status = snd_pcm_wait(_handle, 500)) < 0) {
		return error("Error while waiting for device: ", snd_strerror (status));
	}
	for (int buf=0; buf<8192; buf += _bufSize/4)
		sendFrames(zeroBuffer, _bufSize/4);
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
		printf("ALSAAudioDevice::doStop: waiting for thread to finish...\n");
		stopping(true);		// signals play thread
		if (paused()) {
			_stopDuringPause = true;	// we handle this case as special
			snd_pcm_drop(_handle);
		}
		waitForThread();
	}
	return 0;
}

int ALSAAudioDevice::doSetFormat(int sampfmt, int chans, double srate)
{
	int status;
	printf("ALSAAudioDevice::doSetFormat: fmt: %d chans: %d rate: %g\n",
		   sampfmt, chans, srate);
	snd_pcm_format_t sampleFormat = SND_PCM_FORMAT_UNKNOWN;
	switch(sampfmt) {
		case MUS_UBYTE:
			sampleFormat = SND_PCM_FORMAT_U8;
			break;
		case MUS_BYTE:
			sampleFormat = SND_PCM_FORMAT_S8;
			break;
		case MUS_LSHORT:
			sampleFormat = SND_PCM_FORMAT_S16_LE;
			break;
		case MUS_BSHORT:
			sampleFormat = SND_PCM_FORMAT_S16_BE;
			break;
		case MUS_L24INT:
			sampleFormat = SND_PCM_FORMAT_S24_LE;
			break;
		case MUS_B24INT:
			sampleFormat = SND_PCM_FORMAT_S24_BE;
			break;
		case MUS_LINT:
			sampleFormat = SND_PCM_FORMAT_S32_LE;
			break;
		case MUS_BINT:
			sampleFormat = SND_PCM_FORMAT_S32_BE;
			break;
		case MUS_LFLOAT:
			sampleFormat = SND_PCM_FORMAT_FLOAT_LE;
			break;
		case MUS_BFLOAT:
			sampleFormat = SND_PCM_FORMAT_FLOAT_BE;
			break;
		default:
			return error("Unsupported sample format");
	};
	if ((status = snd_pcm_hw_params_set_format(_handle,
											   _hwParams, 
											   sampleFormat)) < 0) {
		return error("Cannot set sample format: ", snd_strerror(status));
	}
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
	return 0;
}

int ALSAAudioDevice::doSetQueueSize(int *pQueueSize)
{
	_bufSize = (snd_pcm_uframes_t) *pQueueSize;
	int status;
 	if ((status = snd_pcm_hw_params_set_buffer_size_near(_handle,
														 _hwParams,
														 &_bufSize)) < 0)
	{
 		return error("Cannot set buffer size: ", snd_strerror(status));
 	}
	printf("ALSAAudioDevice::doSetQueueSize: requested %d frames, got %u\n",
			*pQueueSize,  (unsigned) _bufSize);
	
	int dir = 0;
	snd_pcm_uframes_t periodsize = _bufSize/4, tryperiodsize;

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

// 	if ((status = snd_pcm_hw_params_get_buffer_size(_hwParams, &bufSize)) < 0) {
//		return error("Cannot retrieve buffer size: ", snd_strerror(status));
//	}
	*pQueueSize = _bufSize;
	return getstatus(status);
}

int	ALSAAudioDevice::doGetFrames(void *frameBuffer, int frameCount)
{
	int fread = snd_pcm_readi(_handle, frameBuffer, frameCount);
	if (fread > 0) {
		if (fread < frameCount)
			printf("ALSAAudioDevice::doGetFrames: read %d out of %d frames!\n",
				   fread, frameCount);
		incrementFrameCount(fread);
		return fread;
	}
	else if (fread == -EPIPE) {
		printf("ALSAAudioDevice::doGetFrames: underrun on read -- recovering and calling again\n");
		snd_pcm_prepare(_handle);
		fread = snd_pcm_readi(_handle, frameBuffer, frameCount);
		incrementFrameCount((fread >= 0) ? fread : 0);
		return (fread >= 0) ? 0 : -1;
	}
	else {
		fprintf(stderr, "ALSAAudioDevice::doGetFrames: error reading from device: %s\n", snd_strerror(fread));
		return error("Error reading from device: ", snd_strerror(fread));
	}
	return -1;
}

int	ALSAAudioDevice::doSendFrames(void *frameBuffer, int frameCount)
{
	int fwritten = snd_pcm_writei(_handle, frameBuffer, frameCount);
//	printf("ALSAAudioDevice::doSendFrames: %d frames to write, %d written\n", frameCount, fwritten);
	if (fwritten > 0) {
		if (fwritten < frameCount)
			printf("ALSAAudioDevice::doSendFrames: write %d out of %d frames!\n",
				   fwritten, frameCount);
		incrementFrameCount(fwritten);
		return fwritten;
	}
	else if (fwritten == -EPIPE) {
		printf("ALSAAudioDevice::doSendFrames: underrun on write -- recovering and calling again\n");
		snd_pcm_prepare(_handle);
		fwritten = snd_pcm_writei(_handle, frameBuffer, frameCount);
		incrementFrameCount((fwritten >= 0) ? fwritten : 0);
		return (fwritten >= 0) ? 0 : -1;
	}
	else {
		fprintf(stderr, "ALSAAudioDevice::doSendFrames: error writing to device: %s\n", snd_strerror(fwritten));
		return error("Error writing to device: ", snd_strerror(fwritten));
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

AudioDevice *createAudioDevice()
{
	return new ALSAAudioDevice;
}

#endif	// ALSA
