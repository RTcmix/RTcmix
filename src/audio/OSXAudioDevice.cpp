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
		errstring = "Unsuported audio format.";
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
	unsigned int 				deviceBufSamps;			// hw size in samps
	int							deviceChannels;			// hw value
	int							channels;				// what user req's
	int							frameCount;
	bool						formatWritable;			// true if device allows changes to fmt
	bool						paused;
	bool						stopping;
	bool						recording;				// Used by OSX code
	bool						playing;				// Used by OSX code
	float						*audioBuffers[2];		// circ. buffers
	int							audioBufSamps;			// length of buffers in samps
	int							inLoc[2], outLoc[2];	// circ. buffer indices
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
	// Samps, not frames.
	const int bufLen = impl->audioBufSamps;
	// How many samples are available from HW.
	const int sampsToRead = impl->deviceBufSamps;
	if (impl->recording) {
		// How many samples' space are available in our buffer.
		int spaceAvail = ::inRemaining(impl->outLoc[REC], impl->inLoc[REC], bufLen);

		// printf("OSXAudioDevice: record section\n");
		// printf("sampsToRead = %d, spaceAvail = %d\n", sampsToRead, spaceAvail);

		// Run this loop while not enough space to copy audio from HW.
		while (spaceAvail < sampsToRead && keepGoing) {
			Callback runCallback = device->getRunCallback();
			keepGoing = (*runCallback)(device, device->getRunCallbackContext());
			spaceAvail = ::inRemaining(impl->outLoc[REC], impl->inLoc[REC], bufLen);
			// printf("\tafter run callback, spaceAvail = %d\n", spaceAvail);
		}
		if (keepGoing == true) {
			// printf("\tinLoc begins at %d (out of %d)\n",
			// 		  impl->inLoc[REC], bufLen);
			int	sampsCopied = 0;
			// Write new audio data into audioBuffers[REC].
			//   Treat it as circular buffer.
			while (sampsCopied < sampsToRead) {
				register float *inbuf = (float *) inInputData->mBuffers[0].mData;
				register float *outbuf = impl->audioBuffers[REC];
				int inLoc = impl->inLoc[REC];
				for (int n = 0; n < sampsToRead; ++n) {
					if (inLoc == bufLen)	// wrap
						inLoc = 0;
					inbuf[n] = outbuf[inLoc++];
				}
				impl->inLoc[REC] = inLoc;
				sampsCopied = sampsToRead;
			}
			// printf("\tinLoc ended at %d\n", impl->inLoc[REC]);
		}
		else if (!impl->playing) {
			// printf("OSXAudioDevice: run callback returned false -- calling stop callback\n");
			Callback stopCallback = device->getStopCallback();
			if (stopCallback) {
				(*stopCallback)(device, device->getStopCallbackContext());
			}
			device->close();
			// printf("OSXAudioDevice: leaving runProcess\n");
			return kAudioHardwareNoError;
		}
	}
	if (impl->playing) {
		// Samps, not frames.
		const int sampsToWrite = impl->deviceBufSamps;
		int sampsAvail = ::outRemaining(impl->inLoc[PLAY], impl->outLoc[PLAY], bufLen);

		// printf("OSXAudioDevice: playback section\n");
		// printf("sampsAvail = %d\n", sampsAvail);
		while (sampsAvail < sampsToWrite && keepGoing) {
			// printf("\tsampsAvail < needed (%d), so run callback for more\n", sampsToWrite);
			Callback runCallback = device->getRunCallback();
			keepGoing = (*runCallback)(device, device->getRunCallbackContext());
			sampsAvail = ::outRemaining(impl->inLoc[PLAY], impl->outLoc[PLAY], bufLen);
			// printf("\tafter run callback, sampsAvail = %d\n", sampsAvail);
		}
		if (keepGoing == true) {
			// printf("\toutLoc begins at %d (out of %d)\n",
			//	   impl->outLoc[PLAY], bufLen);
			int samplesDone = 0;
			// Audio data has been written into audioBuffers[PLAY] during doSendFrames.
			//   Treat it as circular buffer.
			while (samplesDone < sampsToWrite) {
				register float *inbuf = impl->audioBuffers[PLAY];
				register float *outbuf = (float *) outOutputData->mBuffers[0].mData;
				int outLoc = impl->outLoc[PLAY];
				for (int n = 0; n < sampsToWrite; ++n) {
					if (outLoc == bufLen)	// wrap
						outLoc = 0;
					outbuf[n] = inbuf[outLoc++];
				}
				impl->outLoc[PLAY] = outLoc;
				samplesDone += sampsToWrite;
			}
			// printf("\toutLoc ended at %d\n", impl->outLoc[PLAY]);
		}
		else {
			// printf("OSXAudioDevice: run callback returned false -- calling stop callback\n");
			Callback stopCallback = device->getStopCallback();
			if (stopCallback) {
				(*stopCallback)(device, device->getStopCallbackContext());
			}
			device->close();
		}
	}
	impl->frameCount += sampsToRead / impl->deviceChannels;
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
		Callback stopCallback = device->getStopCallback();
		if (stopCallback) {
			(*stopCallback)(device, device->getStopCallbackContext());
		}
	}
	return err;
}

OSXAudioDevice::OSXAudioDevice() : _impl(new Impl)
{
	_impl->deviceID = 0;
	_impl->bufferSampleFormat = MUS_UNKNOWN;
	_impl->deviceBufSamps = 0;
	_impl->deviceChannels = 0;
	_impl->audioBufSamps = 0;
	_impl->channels = 0;
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
		return error("Can't get audio device writeable property: ", errToString(err));
	}
	_impl->formatWritable = (writeable != 0);
	// Register our callback functions with the HAL.
	err = AudioDeviceAddPropertyListener(_impl->deviceID,
										0, _impl->recording,
										kAudioDevicePropertyDeviceIsRunning,
									   _impl->listenerProcess, 
									   (void *) this);
	if (err != kAudioHardwareNoError) {
		return error("Cannot register property listener with device: ", errToString(err));
	}
	err = AudioDeviceAddIOProc(_impl->deviceID,
							   _impl->runProcess, 
							   (void *) this);
	if (err != kAudioHardwareNoError) {
		return error("Cannot register callback function with device: ", errToString(err));
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
	// Sanity check, because we do the conversion to float ourselves.
	if (fmt != MUS_BFLOAT && fmt != MUS_BSHORT)
		return error("Only 16bit and float audio buffers supported.");

	_impl->bufferSampleFormat = fmt;
	// We catch mono input and do the conversion ourselves.
	_impl->channels = (chans == 1) ? 2 : chans;
	if (_impl->formatWritable)
	{
		// Default all values to device's defaults (from doOpen()), then set
		// our sample rate and channel count.
		AudioStreamBasicDescription requestedFormat = _impl->deviceFormat;
		requestedFormat.mSampleRate = srate;
		requestedFormat.mChannelsPerFrame = _impl->channels;
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
			return error("Can't retrieve audio device format: ", errToString(err));
		}
		else if (_impl->deviceFormat.mChannelsPerFrame != _impl->channels) {
			return error("This channel count not supported.");
		}
		else if (_impl->deviceFormat.mSampleRate != srate) {
			return error("This sampling rate not supported.");
		}
		// We are OK.  Cache retrieved device channel count.
		_impl->deviceChannels = _impl->deviceFormat.mChannelsPerFrame;
		return 0;
	}
	// If format was not writable, see if our request matches defaults.
	else if (_impl->deviceFormat.mSampleRate != srate) {
		return error("Audio srate not writable on this device");
	}
	else if (_impl->deviceFormat.mChannelsPerFrame != _impl->channels) {
		return error("Audio channel count not writable on this device");
	}
	return 0;
}

int OSXAudioDevice::doSetQueueSize(int *pQueueSize)
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
	int reqQueueFrames = *pQueueSize;
	// Audio buffer is always floating point.  Attempt to set size in bytes.
	// Loop until request is accepted, halving value each time.
	unsigned int reqBufBytes = sizeof(float) * _impl->channels * reqQueueFrames;
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
		_impl->audioBufSamps = 2 * deviceFrames * _impl->deviceChannels;
	}
	else {
		_impl->audioBufSamps = reqQueueFrames * _impl->deviceChannels;
	}

	_impl->deviceBufSamps = deviceBufferBytes / sizeof(float);	// in samples, not frames


	printf("device bufsize: %d bytes (%d frames). circ buffer %d frames\n",
			deviceBufferBytes, deviceFrames, _impl->audioBufSamps/_impl->deviceChannels);

	delete [] _impl->audioBuffers[REC];  _impl->audioBuffers[REC] = NULL;
	delete [] _impl->audioBuffers[PLAY]; _impl->audioBuffers[PLAY] = NULL;
	if (_impl->recording) {
		_impl->audioBuffers[REC] = new float[_impl->audioBufSamps];
		if (_impl->audioBuffers[REC] == NULL)
			return error("Memory allocation failure for OSXAudioDevice buffer!");
		_impl->inLoc[REC] = _impl->outLoc[REC] = 0;
	}
	if (_impl->playing) {
		_impl->audioBuffers[PLAY] = new float[_impl->audioBufSamps];
		if (_impl->audioBuffers[PLAY] == NULL)
			return error("Memory allocation failure for OSXAudioDevice buffer!");
		_impl->inLoc[PLAY] = _impl->outLoc[PLAY] = 0;
	}
	return 0;
}

int	OSXAudioDevice::doGetFrames(void *frameBuffer, int frameCount)
{
	const int chans = _impl->channels;
	float *from = _impl->audioBuffers[REC];
	const int bufLen = _impl->audioBufSamps;
	int outLoc = _impl->outLoc[REC];
	// printf("OSXAudioDevice::doGetFrames: frameCount = %d\n", frameCount);
	// printf("\toutLoc begins at %d (out of %d)\n", outLoc, bufLen);

	float scale;
	switch (chans) {
	case 1:
		switch (_impl->bufferSampleFormat) {
		case MUS_BSHORT:
			{
				scale = 32768.0 * 0.707;
				short *outbuf = (short *) frameBuffer;
				for (int out=0; out < frameCount; ++out) {
					if (outLoc >= bufLen)	// wrap
						outLoc -= bufLen;
					outbuf[out] = (short) (from[outLoc] * scale);	
					outbuf[out] += (short) (from[outLoc+1] * scale);	
					outLoc += 2;
				}
			}
			break;
		case MUS_BFLOAT:
			{
				scale = 32768.0 * 0.707;
				float *outbuf = (float *) frameBuffer;
				for (int out=0; out < frameCount; ++out) {
					if (outLoc >= bufLen)	// wrap
						outLoc -= bufLen;
					outbuf[out] = from[outLoc] * scale;	
					outbuf[out] += from[outLoc+1] * scale;	
					outLoc += 2;
				}
			}
			break;
		default:
			return error("This sample format conversion not supported");
		}
		break;

	case 2:
		switch (_impl->bufferSampleFormat) {
		case MUS_BSHORT:
			{
				scale = 32768.0;
				short *outbuf = (short *) frameBuffer;
				for (int out=0; out < frameCount; ++out) {
					if (outLoc >= bufLen)	// wrap
						outLoc -= bufLen;
					outbuf[out] = (short) (from[outLoc] * scale);	
					outbuf[out+1] = (short) (from[outLoc+1] * scale);	
					outLoc += 2;
				}
			}
			break;
		case MUS_BFLOAT:
			{
				scale = 32768.0;
				float *outbuf = (float *) frameBuffer;
				for (int out=0; out < frameCount; ++out) {
					if (outLoc >= bufLen)
						outLoc -= bufLen;	// wrap
					outbuf[out] = from[outLoc] * scale;	
					outbuf[out+1] = from[outLoc+1] * scale;	
					outLoc += 2;
				}
			}
			break;
		default:
			return error("This sample format conversion not supported");
		}
		break;

	default:
		return error("This channel count not yet supported by format conversion code");
	}
	_impl->outLoc[REC] = outLoc;
	// printf("\toutLoc ended at %d.  Returning frameCount = %d\n", outLoc, frameCount);
	return frameCount;
}

int	OSXAudioDevice::doSendFrames(void *frameBuffer, int frameCount)
{
	const int chans = _impl->channels;
	float *outbuf = _impl->audioBuffers[PLAY];
	const int bufLen = _impl->audioBufSamps;
	int inLoc = _impl->inLoc[PLAY];
	//printf("OSXAudioDevice::doSendFrames: frameCount = %d\n", frameCount);
	// printf("\tinLoc begins at %d (out of %d)\n", inLoc, bufLen);
	float scale;
	switch (chans) {
	case 1:
		switch (_impl->bufferSampleFormat) {
		case MUS_BSHORT:	// mono short to stereo float
			{
				scale = 1.0/32768.0;
				short *from = (short *) frameBuffer;
				for (int in=0; in < frameCount; ++in) {
					if (inLoc >= bufLen)	// wrap
						inLoc -= bufLen;
					outbuf[inLoc] = (float)(*from * scale);	
					outbuf[inLoc+1] = (float)(*from * scale);	
					++from;
					inLoc += 2;
				}
			}
			break;
		case MUS_BFLOAT:	// mono float to stereo float
			{
				scale = 1.0/32768.0;
				float *from = (float *) frameBuffer;
				for (int in=0; in < frameCount; ++in) {
					if (inLoc >= bufLen)	// wrap
						inLoc -= bufLen;
					outbuf[inLoc] = (float)(*from * scale);	
					outbuf[inLoc+1] = (float)(*from * scale);	
					++from;
					inLoc += 2;
				}
			}
			break;
		default:
			return error("This sample format conversion not supported");
		}
		break;

	case 2:
		switch (_impl->bufferSampleFormat) {
		case MUS_BSHORT:	// stereo short to stereo float
			{
				scale = 1.0/32768.0;
				short *from = (short *) frameBuffer;
				for (int in=0; in < frameCount; ++in) {
					if (inLoc >= bufLen)	// wrap
						inLoc -= bufLen;
					outbuf[inLoc] = (float)(from[0] * scale);	
					outbuf[inLoc+1] = (float)(from[1] * scale);	
					from += 2;
					inLoc += 2;
				}
			}
			break;
		case MUS_BFLOAT:	// stereo float to stereo float (rescaled)
			{
				scale = 1.0/32768.0;
				float *from = (float *) frameBuffer;
				for (int in=0; in < frameCount; ++in) {
					if (inLoc >= bufLen)
						inLoc -= bufLen;	// wrap
					outbuf[inLoc] = (float)(from[0] * scale);	
					outbuf[inLoc+1] = (float)(from[1] * scale);	
					from += 2;
					inLoc += 2;
				}
			}
			break;
		default:
			return error("This sample format conversion not supported");
		}
		break;

	default:
		return error("This channel count not yet supported by format conversion code");
	}
	_impl->inLoc[PLAY] = inLoc;
	// printf("\tinLoc ended at %d.  Returning frameCount = %d\n", inLoc, frameCount);
	return frameCount;
}

int OSXAudioDevice::doGetFrameCount() const
{
	return _impl->frameCount;
}

AudioDevice *createAudioDevice()
{
	return new OSXAudioDevice;
}

#endif	// MACOSX
