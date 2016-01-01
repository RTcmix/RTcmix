//
//  EmbeddedAudioDevice.cpp
//  RTcmix
//
//  Created by Douglas Scott on 12/30/15.
//
//

#include "EmbeddedAudioDevice.h"
#include "sndlibsupport.h"
#include <string.h>

static int sCallbackAudioFormat;
static int sCallbackAudioChannels;

int SetEmbeddedCallbackAudioFormat(int sampfmt, int chans)
{
	sCallbackAudioFormat = sampfmt;
	sCallbackAudioChannels = chans;
	return 0;
}

struct EmbeddedAudioDevice::Impl {
	Impl() : inputAudio(NULL), outputAudio(NULL), audioFormat(0), audioChannels(0), sampleSize(0), frameCount(0) {}
	void *inputAudio;
	void *outputAudio;
	// Format of above audio, as passed in via run()
	int audioFormat;
	int audioChannels;
	int sampleSize;	// cached
	int	frameCount;
};


EmbeddedAudioDevice::EmbeddedAudioDevice() : _impl(new Impl)
{
}

EmbeddedAudioDevice::~EmbeddedAudioDevice()
{
	close();
	delete _impl;
}

bool EmbeddedAudioDevice::run(void *inputFrameBuffer, void *outputFrameBuffer, int frameCount)
{
	_impl->inputAudio = inputFrameBuffer;
	_impl->outputAudio = outputFrameBuffer;
	return runCallback();
}

int EmbeddedAudioDevice::doOpen(int mode)
{
	// For now, we don't care -- we only copy to/from buffers that are non-null
	switch (mode & DirectionMask) {
		case Playback:
			break;
		case Record:
			break;
		case RecordPlayback:
			break;
		default:
			error("AudioDevice: Illegal open mode.");
	}
	return 0;
}

// doClose() is called by AudioDevice::close() to do the class-specific closing
// of the audio port, HW, device, etc.
// You are guaranteed that doClose() will NOT be called if you are already closed.

int EmbeddedAudioDevice::doClose()
{
	_impl->frameCount = 0;
	return 0;
}

// doStart() is called by AudioDevice::start() to do class-specific calls which
// notify the HW to begin recording, playing, or both.

int EmbeddedAudioDevice::doStart()
{
	return 0;
}

int EmbeddedAudioDevice::doStop()
{
	_impl->frameCount = 0;
	return 0;
}

// This does nothing under RTcmix, so can be left as-is.

int EmbeddedAudioDevice::doPause(bool)
{
	return error("This audio device cannot be paused");
}

// doSetFormat() is called by AudioDevice::setFormat() and by AudioDevice::open().
// Here is where you configure your HW, setting it to the format which will
// best handle the format passed in.  Note that it is NOT necessary for the HW
// to match the input format except in sampling rate;  The base class can handle
// most format conversions.
// 'sampfmt' is the format of the data passed to AudioDevice::getFrames() or
//	AudioDevice::sendFrames(), and has three attributes:
//	1) The actual type of the format, retrieved via MUS_GET_FORMAT(sampfmt)
//	2) The interleave (true or false) retrieved via MUS_GET_INTERLEAVE(sampfmt)
//	3) Whether the samples (when float) are normalized, retrieved via
//		MUS_GET_NORMALIZED(sampfmt)
//
// At the end of this method, you must call setDeviceParams() to notify the
// base class what format *you* need the audio data to be in.

int EmbeddedAudioDevice::doSetFormat(int sampfmt, int chans, double srate)
{
	_impl->audioFormat = sCallbackAudioFormat;
	_impl->audioChannels = sCallbackAudioChannels;
	switch (MUS_GET_FORMAT(sampfmt)) {
		case NATIVE_SHORT_FMT:
			_impl->sampleSize = 2;
			break;
		case NATIVE_24BIT_FMT:
			_impl->sampleSize = 3;
			break;
		case NATIVE_32BIT_FMT:
			_impl->sampleSize = 4;
			break;
		case NATIVE_FLOAT_FMT:
			_impl->sampleSize = 4;
			break;
		default:
			return error("Unknown audio format for external buffers");
	}
	setDeviceParams(_impl->audioFormat, _impl->audioChannels, srate);
	return 0;
}

// doSetQueueSize() is called by AudioDevice::setQueueSize() to allow HW-specific
// configuration of internal audio queue sizes.  The values handed in via
// address represent the size **in frames** of the buffers that will be handed
// to doGetFrames() and/or doSendFrames(), and the number of such buffers the
// application would like to have queued up for robustness.  The actual frame
// count as determined by your HW *must* be reported back to the caller via
// 'pWriteSize'.  If you cannot match *pCount, just do the best you can, but
// do not fail if you cannot match it.

int EmbeddedAudioDevice::doSetQueueSize(int *pWriteSize, int *pCount)
{
	return 0;
}

int EmbeddedAudioDevice::doGetFrameCount() const
{
	return _impl->frameCount;
}

// doGetFrames() is called by AudioDevice::getFrames() during record.
// The format of 'frameBuffer' will be the format **YOU** specified via
// setDeviceParams() above.  It will be converted into the 'frame format'
// by a base class.  Here is where you fill frameBuffer from your audio HW.

int EmbeddedAudioDevice::doGetFrames(void *frameBuffer, int frameCount)
{
	if (!_impl->inputAudio)
		return 0;

	if (MUS_GET_INTERLEAVE(_impl->audioFormat) == MUS_INTERLEAVED) {
		int bytesPerFrame = _impl->sampleSize * _impl->audioChannels;
		// Copy audio from the cached external audio buffer pointer to the buffer pointed to by 'frameBuffer'
		memcpy(frameBuffer, _impl->inputAudio, frameCount * bytesPerFrame);
	}
	else {
		return error("Non-interleaved audio not yet handled");
	}
	return frameCount;
}

// doSendFrames() is called by AudioDevice::sendFrames() during playback.
// The format of 'frameBuffer' will be the format **YOU** specified via
// setDeviceParams() above.  It was converted from the 'frame format'
// by a base class.   Here is where you hand the audio in frameBuffer to you
// HW.

int EmbeddedAudioDevice::doSendFrames(void *frameBuffer, int frameCount)
{
	if (MUS_GET_INTERLEAVE(_impl->audioFormat) == MUS_INTERLEAVED) {
		int bytesPerFrame = _impl->sampleSize * _impl->audioChannels;
		// Copy audio from the pointer to the buffer pointed to by 'frameBuffer' to the cached external audio buffer
		memcpy(_impl->outputAudio, frameBuffer, frameCount * bytesPerFrame);
	}
	else {
		return error("Non-interleaved audio not yet handled");
	}
	_impl->frameCount += frameCount;
	return frameCount;
}

// Return true if the passed in device descriptor matches one that this device
// can understand.  In this case, we always return true.

bool EmbeddedAudioDevice::recognize(const char *desc)
{
	return true;
}

// If your audio device(s) needs a string descriptor, it will come in via 'inputDesc'
// and/or 'outputDesc', allowing you to specify different HW for record and play.

AudioDevice *EmbeddedAudioDevice::create(const char *inputDesc, const char *outputDesc, int mode)
{
	return new EmbeddedAudioDevice;
}
