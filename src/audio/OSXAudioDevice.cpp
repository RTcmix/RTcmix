// OSXAudioDevice.cpp

#if defined(MACOSX)

#include "OSXAudioDevice.h"
#include <CoreAudio/CoreAudio.h>
#include <mach/mach.h>
#include <mach/policy.h>
#include <pthread.h>
#include <stdio.h>
#include <sndlibsupport.h>	// RTcmix header

const char *errToString(OSStatus err)
{
	const char *errstring;
	switch (err) {
	case kAudioHardwareNoError:
		errstring = "No error.";
		break;
	case kAudioHardwareNotRunningError:
		errstring = "Hardware not running.";
		break;
	case kAudioHardwareUnspecifiedError:
		errstring = "Unspecified error.";
		break;
	case kAudioHardwareUnknownPropertyError:
		errstring = "Unknown hardware property.";
		break;
	case kAudioDeviceUnsupportedFormatError:
		errstring = "Unsupported audio format.";
		break;
	case kAudioHardwareBadPropertySizeError:
		errstring = "Bad hardware propery size.";
		break;
	case kAudioHardwareIllegalOperationError:
		errstring = "Illegal operation.";
		break;
	default:
		errstring = "Unknown error.";
	}
	return errstring;
}

static const int REC = 0, PLAY = 1;

struct OSXAudioDevice::Impl {
	AudioDeviceID				deviceID;
	AudioStreamBasicDescription deviceFormat;
	int							bufferSampleFormat;
	unsigned int 				deviceBufFrames;		// hw buf length
	int							deviceChannels;			// hw value
	int							frameCount;
	bool						formatWritable;			// true if device allows changes to fmt
	bool						paused;
	bool						stopping;
	bool						recording;				// Used by OSX code
	bool						playing;				// Used by OSX code
	float						*audioBuffers[2];		// circ. buffers
	int							audioBufFrames;			// length of audioBuffers
	int							audioBufChannels;		// channels in audioBuffers
	int							inLoc[2],
								outLoc[2];	// circ. buffer indices
	static OSStatus				runProcess(
									AudioDeviceID			inDevice,
									const AudioTimeStamp	*inNow,
									const AudioBufferList	*inInputData,
									const AudioTimeStamp	*inInputTime,
									AudioBufferList			*outOutputData,
									const AudioTimeStamp	*inOutputTime,
							  		void					*object);
	static OSStatus				listenerProcess(
									AudioDeviceID inDevice,
									UInt32 inChannel,
									Boolean isInput,
									AudioDevicePropertyID inPropertyID,
									void *object);
									
};

inline int min(int x, int y) { return (x <= y) ? x : y; }

inline int inRemaining(int in, int out, int size) {
	return (in <= out) ? in + size - out : in - out;
}

inline int outRemaining(int in, int out, int size) {
	return (in < out) ? in + size - out : in - out;
}

OSStatus
OSXAudioDevice::Impl::runProcess(AudioDeviceID			inDevice,
						  		  const AudioTimeStamp	*inNow,
								  const AudioBufferList	*inInputData,
								  const AudioTimeStamp	*inInputTime,
								  AudioBufferList		*outOutputData,
								  const AudioTimeStamp	*inOutputTime,
								  void					*object)
{
	OSXAudioDevice *device = (OSXAudioDevice *) object;
	OSXAudioDevice::Impl *impl = device->_impl;
	// printf("OSXAudioDevice: top of runProcess\n");
	bool keepGoing = true;
	if (impl->recording) {
		const int destframes = impl->audioBufFrames;
		const int destchans = impl->deviceChannels;
		const int bufLen = destframes * destchans;
		// How many frames are available from HW.
		const int framesToRead = impl->deviceBufFrames;
		// How many frames' space are available in our buffer.
		int framesAvail = ::inRemaining(impl->outLoc[REC], impl->inLoc[REC], bufLen) / destchans;

//		printf("OSXAudioDevice: record section\n");
//		printf("framesToRead = %d, framesAvail = %d\n", framesToRead, framesAvail);

		// Run this loop while not enough space to copy audio from HW.
		while (framesAvail < framesToRead && keepGoing) {
			keepGoing = device->runCallback();
			framesAvail = ::inRemaining(impl->outLoc[REC], impl->inLoc[REC], bufLen) / destchans;
//			printf("\tafter run callback, framesAvail = %d\n", framesAvail);
		}
		if (keepGoing == true) {
//			printf("\tinLoc begins at %d (out of %d)\n",
//			 		  impl->inLoc[REC], bufLen);
			int	framesCopied = 0;
			// Write new audio data into audioBuffers[REC].
			//   Treat it as circular buffer.
			while (framesCopied < framesToRead) {
				register float *src = (float *) inInputData->mBuffers[0].mData;
				register float *dest = impl->audioBuffers[REC];
				int inLoc = impl->inLoc[REC];
				for (int n = 0; n < framesToRead; ++n) {
					if (inLoc == bufLen)	// wrap
						inLoc = 0;
					for (int ch = 0; ch < destchans; ++ch) {
						dest[inLoc++] = src[ch];
					}
					src += destchans;
				}
				impl->inLoc[REC] = inLoc;
				framesCopied = framesToRead;
			}
//			printf("\tinLoc ended at %d\n", impl->inLoc[REC]);
		}
		else if (!impl->playing) {
			// printf("OSXAudioDevice: run callback returned false -- calling stop callback\n");
			device->stopCallback();
			device->close();
//			printf("OSXAudioDevice: leaving runProcess\n");
			return kAudioHardwareNoError;
		}
	}
	if (impl->playing) {
		// Samps, not frames.
		const int framesToWrite = impl->deviceBufFrames;
		const int srcframes = impl->audioBufFrames;
		const int srcchans = impl->audioBufChannels;	// THIS CAN BE LESS THAN 'destchans'
		const int bufLen = srcframes * srcchans;
		const int destchans = impl->deviceChannels;
		const int chansToCopy = (srcchans < destchans) ? srcchans : destchans;

		int framesAvail = ::outRemaining(impl->inLoc[PLAY], impl->outLoc[PLAY], bufLen) / srcchans;
		
//		printf("OSXAudioDevice: playback section\n");
//		printf("framesAvail = %d\n", framesAvail);
		while (framesAvail < framesToWrite && keepGoing) {
//			printf("\tframesAvail < needed (%d), so run callback for more\n", framesToWrite);
			keepGoing = device->runCallback();
			framesAvail = ::outRemaining(impl->inLoc[PLAY], impl->outLoc[PLAY], bufLen) / srcchans;
//			printf("\tafter run callback, framesAvail = %d\n", framesAvail);
		}
		if (keepGoing == true) {
//			printf("\toutLoc begins at %d (out of %d)\n",
//				   impl->outLoc[PLAY], bufLen);
			int framesDone = 0;
			// Audio data has been written into audioBuffers[PLAY] during doSendFrames.
			//   Treat it as circular buffer.
			while (framesDone < framesToWrite) {
				register float *src = impl->audioBuffers[PLAY];
				register float *dest = (float *) outOutputData->mBuffers[0].mData;
				int outLoc = impl->outLoc[PLAY];
				for (int n = 0; n < framesToWrite; ++n) {
					if (outLoc == bufLen)	// wrap
						outLoc = 0;
					for (int ch = 0; ch < destchans; ++ch) {
						if (ch < chansToCopy)
							dest[ch] = src[outLoc++];
						else
							dest[ch] = 0.0f;
					}
					dest += destchans;
				}
				impl->outLoc[PLAY] = outLoc;
				framesDone += framesToWrite;
			}
//			printf("\toutLoc ended at %d\n", impl->outLoc[PLAY]);
		}
		else {
			// printf("OSXAudioDevice: run callback returned false -- calling stop callback\n");
			device->stopCallback();
			device->close();
		}
	}
	impl->frameCount += impl->deviceBufFrames;
//	printf("OSXAudioDevice: leaving runProcess\n\n");
	return kAudioHardwareNoError;
}

OSStatus
OSXAudioDevice::Impl::listenerProcess(AudioDeviceID inDevice,
										UInt32 inChannel,
										Boolean isInput,
										AudioDevicePropertyID inPropertyID,
										void *object)
{
	OSXAudioDevice *device = (OSXAudioDevice *) object;
	OSXAudioDevice::Impl *impl = device->_impl;
	OSStatus err = noErr;
	Boolean isRunning = 1;
	UInt32 size = sizeof(isRunning);
	// printf("OSXAudioDevice::Impl::listenerProcess() called\n");
	switch (inPropertyID) {
	case kAudioDevicePropertyDeviceIsRunning:
		err = AudioDeviceGetProperty(
						impl->deviceID,
						0, impl->recording,
						kAudioDevicePropertyDeviceIsRunning,
						&size,
				   		(void *) &isRunning);
		break;
	default:
		// printf("Some other property was changed.\n");
		break;
	}
	if (!isRunning && impl->stopping) {
		impl->stopping = false;	// We only want 1 invocation of callback
		// printf("OSXAudioDevice: no longer running -- calling stop callback\n");
		device->stopCallback();
	}
	return err;
}

OSXAudioDevice::OSXAudioDevice() : _impl(new Impl)
{
	_impl->deviceID = 0;
	_impl->bufferSampleFormat = MUS_UNKNOWN;
	_impl->deviceBufFrames = 0;
	_impl->deviceChannels = 0;
	_impl->audioBufFrames = 0;
	_impl->audioBufChannels = 0;
	_impl->audioBuffers[REC] = _impl->audioBuffers[PLAY] = NULL;
	_impl->frameCount = 0;
	_impl->inLoc[REC] = _impl->inLoc[PLAY] = 0;
	_impl->outLoc[REC] = _impl->outLoc[PLAY] = 0;
	_impl->paused = false;
	_impl->stopping = false;
	_impl->recording = false;
	_impl->playing = false;
}

OSXAudioDevice::~OSXAudioDevice()
{
	//printf("OSXAudioDevice::~OSXAudioDevice()\n");
	delete [] _impl->audioBuffers[REC];
	delete [] _impl->audioBuffers[PLAY];
	delete _impl;
}

int OSXAudioDevice::doOpen(int mode)
{
	AudioDeviceID devID;
	_impl->recording = ((mode & Record) != 0);
	_impl->playing = ((mode & Playback) != 0);
	UInt32 size = sizeof(devID);
	OSStatus err = AudioHardwareGetProperty(
						kAudioHardwarePropertyDefaultOutputDevice,
						&size,
				   		(void *) &devID);
	if (err != kAudioHardwareNoError || devID == kAudioDeviceUnknown) {
		return error("Cannot find default audio device: ", errToString(err));
	}
	_impl->deviceID = devID;
	// Get current output format	
	size = sizeof(_impl->deviceFormat);
	err = AudioDeviceGetProperty(_impl->deviceID, 
								  0, _impl->recording,
								  kAudioDevicePropertyStreamFormat, 
								  &size, 
								  &_impl->deviceFormat);
	if (err != kAudioHardwareNoError) {
		return error("Can't get audio device format: ", errToString(err));
	}
	// Cache this.
	_impl->deviceChannels = _impl->deviceFormat.mChannelsPerFrame;
	// Test and store whether or not audio format property is writable.
	Boolean writeable;
	size = sizeof(writeable);
	err = AudioDeviceGetPropertyInfo(_impl->deviceID, 
   									0, _impl->recording,
								    kAudioDevicePropertyStreamFormat,
									&size,
									&writeable);
	if (err != kAudioHardwareNoError) {
		return error("Can't get audio device writeable property: ", 
					 errToString(err));
	}
	_impl->formatWritable = (writeable != 0);
	// Register our callback functions with the HAL.
	err = AudioDeviceAddPropertyListener(_impl->deviceID,
										0, _impl->recording,
										kAudioDevicePropertyDeviceIsRunning,
									   _impl->listenerProcess, 
									   (void *) this);
	if (err != kAudioHardwareNoError) {
		return error("Cannot register property listener with device: ",
					 errToString(err));
	}
	err = AudioDeviceAddIOProc(_impl->deviceID,
							   _impl->runProcess, 
							   (void *) this);
	if (err != kAudioHardwareNoError) {
		return error("Cannot register callback function with device: ",
					 errToString(err));
	}

	return 0;
}

int OSXAudioDevice::doClose()
{
	// printf("OSXAudioDevice::doClose()\n");
	OSStatus err = AudioDeviceRemoveIOProc(_impl->deviceID, _impl->runProcess);
	int status = (err == kAudioHardwareNoError) ? 0 : -1;
	if (status == -1)
		error("OSXAudioDevice::doClose: error removing IO proc: ",
			  errToString(err));
	err = AudioDeviceRemovePropertyListener(_impl->deviceID,
											0, _impl->recording,
											kAudioDevicePropertyDeviceIsRunning,
										   _impl->listenerProcess);
	status = (err == kAudioHardwareNoError) ? status : -1;
	_impl->frameCount = 0;
	return status;
}

int OSXAudioDevice::doStart()
{
	// printf("OSXAudioDevice::doStart()\n");
	_impl->stopping = false;
	OSStatus err = AudioDeviceStart(_impl->deviceID, _impl->runProcess);
	int status = (err == kAudioHardwareNoError) ? 0 : -1;
	return status;
}

int OSXAudioDevice::doPause(bool pause)
{
	_impl->paused = pause;
	return error("OSXAudioDevice: pause not yet implemented");
}

int OSXAudioDevice::doStop()
{
	// printf("OSXAudioDevice::doStop()\n");
	_impl->stopping = true;	// avoids multiple stop notifications
	OSStatus err = AudioDeviceStop(_impl->deviceID, _impl->runProcess);
	int status = (err == kAudioHardwareNoError) ? 0 : -1;
	return status;
}

int OSXAudioDevice::doSetFormat(int fmt, int chans, double srate)
{
	_impl->bufferSampleFormat = MUS_GET_FORMAT(fmt);

	// Sanity check, because we do the conversion to float ourselves.
	if (_impl->bufferSampleFormat != MUS_BFLOAT)
		return error("Only float audio buffers supported at this time.");

	// We catch mono input and do the conversion ourselves.  Otherwise we
	// create a buffer equal to the requested channel count.
	_impl->audioBufChannels = (chans == 1) ? 2 : chans;
	int requestedChannels = chans;
	if (_impl->formatWritable)
	{
		// Default all values to device's defaults (from doOpen()), then set
		// our sample rate and channel count.
		// NOTE:  For now, dont even try to set channel count on HW.
		AudioStreamBasicDescription requestedFormat = _impl->deviceFormat;
		requestedFormat.mSampleRate = srate;
#ifdef TRY_TO_SET_CHANNELS
		requestedFormat.mChannelsPerFrame = requestedChannels;
		printf("requesting srate %g, %d channels\n", srate, requestedChannels);
#endif
		UInt32 size = sizeof(requestedFormat);
		OSStatus err = AudioDeviceSetProperty(_impl->deviceID,
									 NULL,
									 0, _impl->recording,
								     kAudioDevicePropertyStreamFormat,
									 size,
									 (void *)&requestedFormat);
		if (err != kAudioHardwareNoError) {
			return error("Can't set audio device format: ", errToString(err));
		}
		// Now retrieve settings to see what we got (IS THIS NECESSARY?)
		size = sizeof(_impl->deviceFormat);
		err = AudioDeviceGetProperty(_impl->deviceID, 
									  0, _impl->recording,
									  kAudioDevicePropertyStreamFormat, 
									  &size, 
									  &_impl->deviceFormat);
		if (err != kAudioHardwareNoError) {
			return error("Can't retrieve audio device format: ",
						 errToString(err));
		}
#ifdef TRY_TO_SET_CHANNELS
		else if (_impl->deviceFormat.mChannelsPerFrame != requestedChannels) {
			return error("This channel count not supported.");
		}
#endif
		else if (_impl->deviceFormat.mSampleRate != srate) {
			return error("This sampling rate not supported.");
		}
	}
	// If format was not writable, see if our request matches defaults.
	else if (_impl->deviceFormat.mSampleRate != srate) {
		return error("Audio sample rate not configurable on this device");
	}	
	
	// Cache retrieved device channel count.
	_impl->deviceChannels = _impl->deviceFormat.mChannelsPerFrame;

	int deviceFormat = MUS_BFLOAT | MUS_NORMALIZED;

#ifdef WHEN_NONINTERLEAVED_IS_FINISHED
	// Set the device format based upon settings.  This will be used for format conversion.
	if ((_impl->deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsNonInterleaved) != 0) {
		printf("OSX HW is %d channel, non-interleaved\n", _impl->deviceFormat.mChannelsPerFrame);
		deviceFormat |= MUS_NON_INTERLEAVED;
	}
	else {
		printf("OSX HW is %d channel, interleaved\n", _impl->deviceFormat.mChannelsPerFrame);
		deviceFormat |= MUS_INTERLEAVED;
	}
#else
	// Temporarily, we report back the format of the circular buffers
	// that I use as my intermediate buffers.  This is different from
	// the AudioDeviceImpl's conversion buffer, and involves a second
	// buffer copy, for the time being.

	printf("OSX HW is %d channel, uses temp interleaved buffer\n", _impl->deviceFormat.mChannelsPerFrame);
	deviceFormat |= MUS_INTERLEAVED;
#endif

	setDeviceParams(deviceFormat,
					_impl->deviceFormat.mChannelsPerFrame,
					_impl->deviceFormat.mSampleRate);
	return 0;
}

int OSXAudioDevice::doSetQueueSize(int *pWriteSize, int *pCount)
{
	Boolean writeable;
	UInt32 size = sizeof(writeable);
	OSStatus err = AudioDeviceGetPropertyInfo(_impl->deviceID, 
   									0, _impl->recording,
								    kAudioDevicePropertyBufferSize, 
									&size,
									&writeable);
	if (err != kAudioHardwareNoError) {
		return error("Can't get audio device property");
	}
	int reqQueueFrames = *pWriteSize * *pCount;
	// Audio buffer is always floating point.  Attempt to set size in bytes.
	// Loop until request is accepted, halving value each time.
	unsigned int reqBufBytes = sizeof(float) * _impl->deviceChannels * reqQueueFrames;
	if (writeable) {
		size = sizeof(reqBufBytes);
		while ( (err = AudioDeviceSetProperty(_impl->deviceID,
										 NULL,
										 0, _impl->recording,
										 kAudioDevicePropertyBufferSize,
										 size,
										 (void *)&reqBufBytes))
				!= kAudioHardwareNoError && reqBufBytes > 64)
		{
			reqBufBytes /= 2;
		}
		// Check for total failure.
		if (err != kAudioHardwareNoError) {
			return error("Can't set audio device buffer size to any value");
		}
	}
	// Get and store the actual buffer size.  (Device may not want to change.)
	unsigned int deviceBufferBytes = 0;
	size = sizeof(deviceBufferBytes);
	err = AudioDeviceGetProperty(_impl->deviceID,
								 0, _impl->recording,
								 kAudioDevicePropertyBufferSize,
								 &size,
								 &deviceBufferBytes);
	if (err != kAudioHardwareNoError) {
		return error("Can't get audio device buffer size");
	}
	int deviceFrames = deviceBufferBytes / (sizeof(float) * _impl->deviceChannels);
	printf("OSX device buffer size is %d frames, user req was %d frames\n",
			deviceFrames, reqQueueFrames);

	// We allocate the circular buffers to be the max(2_times_HW, user_req).
	if (deviceFrames * 2 > reqQueueFrames) {
		_impl->audioBufFrames = 2 * deviceFrames;
	}
	else {
		_impl->audioBufFrames = reqQueueFrames;
	}

	_impl->deviceBufFrames = deviceBufferBytes / (_impl->deviceChannels * sizeof(float));


	printf("device bufsize: %d bytes (%d frames). circ buffer %d frames\n",
			deviceBufferBytes, deviceFrames, _impl->audioBufFrames);

	delete [] _impl->audioBuffers[REC];  _impl->audioBuffers[REC] = NULL;
	delete [] _impl->audioBuffers[PLAY]; _impl->audioBuffers[PLAY] = NULL;
	if (_impl->recording) {
		_impl->audioBuffers[REC] = new float[_impl->audioBufFrames * _impl->audioBufChannels];
		if (_impl->audioBuffers[REC] == NULL)
			return error("Memory allocation failure for OSXAudioDevice buffer!");
		_impl->inLoc[REC] = _impl->outLoc[REC] = 0;
	}
	if (_impl->playing) {
		_impl->audioBuffers[PLAY] = new float[_impl->audioBufFrames * _impl->audioBufChannels];
		if (_impl->audioBuffers[PLAY] == NULL)
			return error("Memory allocation failure for OSXAudioDevice buffer!");
		_impl->inLoc[PLAY] = _impl->outLoc[PLAY] = 0;
	}
	return 0;
}

int	OSXAudioDevice::doGetFrames(void *frameBuffer, int frameCount)
{
	const int chans = getFrameChannels();
	float *from = _impl->audioBuffers[REC];
	const int bufLen = _impl->audioBufFrames * _impl->audioBufChannels;
	int outLoc = _impl->outLoc[REC];
	// printf("OSXAudioDevice::doGetFrames: frameCount = %d\n", frameCount);
	// printf("\toutLoc begins at %d (out of %d)\n", outLoc, bufLen);

	switch (chans) {
	case 1:
		if (_impl->audioBufChannels == 2) {
			const float scale = 0.707;
			float *outbuf = (float *) frameBuffer;
			// Combine stereo channels from circ. buffer into mono output frame.
			for (int out=0; out < frameCount; ++out) {
				if (outLoc >= bufLen)	// wrap
					outLoc -= bufLen;
				outbuf[out] = (from[outLoc] + from[outLoc+1]) * scale;	
				outLoc += 2;
			}
		}
		else
			return error("Only stereo-to-mono record conversion is currently supported");
		break;

	default:
		if (_impl->audioBufChannels == getFrameChannels()) {
			float *outbuf = (float *) frameBuffer;
			// Write all channels of each frame from circ. buffer into output frame.
			for (int out=0; out < frameCount; ++out) {
				if (outLoc >= bufLen)
					outLoc -= bufLen;	// wrap
				for (int ch = 0; ch < chans; ++ch) {
					outbuf[ch] = from[outLoc+ch];	
				}
				outLoc += chans;
				outbuf += chans;
			}
		}
		else
			return error("Channel count conversion not supported for record");
		break;
	}
	_impl->outLoc[REC] = outLoc;
	// printf("\toutLoc ended at %d.  Returning frameCount = %d\n", outLoc, frameCount);
	return frameCount;
}

int	OSXAudioDevice::doSendFrames(void *frameBuffer, int frameCount)
{
	const int chans = getFrameChannels();
	float *outbuf = _impl->audioBuffers[PLAY];
	const int bufLen = _impl->audioBufFrames * _impl->audioBufChannels;
	int inLoc = _impl->inLoc[PLAY];
//	printf("OSXAudioDevice::doSendFrames: frameCount = %d\n", frameCount);
//	printf("\tinLoc begins at %d (out of %d)\n", inLoc, bufLen);
	switch (chans) {
	case 1:			// Mono input converted to stereo circ. buffer;  HW 2-N channels.
		if (_impl->audioBufChannels == 2) {
			float *from = (float *) frameBuffer;
			for (int in=0; in < frameCount; ++in) {
				if (inLoc >= bufLen)	// wrap
					inLoc -= bufLen;
				// Write single channel from frame into both chans of circular buf.
				outbuf[inLoc+1] = outbuf[inLoc] = (float)(*from);	
				++from;
				inLoc += 2;
			}
		}
		else
			return error("Only mono-to-stereo playback conversion is currently supported");
		break;

	default:		// 2-N channel input to 2-N channel circ. buffer; HW 2-N channels.
		if (_impl->audioBufChannels == getFrameChannels()) {
			float *from = (float *) frameBuffer;
			// Write all channels of each frame from frame into circular buf.
			for (int in=0; in < frameCount; ++in) {
				if (inLoc >= bufLen)
					inLoc -= bufLen;	// wrap
				for (int ch = 0; ch < chans; ++ch) {
					outbuf[inLoc+ch] = (float) from[ch];	
				}
				from += chans;
				inLoc += chans;
			}
		}
		else
			return error("Channel count conversion not supported for playback");
		break;
	}
	_impl->inLoc[PLAY] = inLoc;
//	printf("\tinLoc ended at %d.  Returning frameCount = %d\n", inLoc, frameCount);
	return frameCount;
}

int OSXAudioDevice::doGetFrameCount() const
{
	return _impl->frameCount;
}

AudioDevice *createAudioDevice(const char *, const char *, bool)
{
	return new OSXAudioDevice;
}

#endif	// MACOSX
