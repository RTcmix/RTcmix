// OSXAudioDevice.cpp

#if defined(MACOSX)

#include "OSXAudioDevice.h"
#include <CoreAudio/CoreAudio.h>
#include <mach/mach.h>
#include <mach/policy.h>
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
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
static const int kMasterChannel = 0;

struct OSXAudioDevice::Impl {
	AudioDeviceID				deviceID;
	struct Port {
		int							streamIndex;		// Which stream
		int							streamChannel;		// 1st chan of that stream
		AudioBufferList				*streamDesc;		// Properties for all strams
		AudioStreamBasicDescription deviceFormat;		// format
		unsigned int 				deviceBufFrames;	// hw buf length
		float						*audioBuffer;		// circ. buffer
		int							audioBufFrames;		// length of audioBuffers
		int							audioBufChannels;	// channels in audioBuffers
		int							inLoc, outLoc;		// circ. buffer indices
		int							audioBufFilled;		// audioBuffer samples available
	} 							port[2];
	
	int							bufferSampleFormat;
	int							frameCount;
	bool						formatWritable;			// true if device allows changes to fmt
	bool						paused;
	bool						stopping;
	bool						recording;				// Used by OSX code
	bool						playing;				// Used by OSX code
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

	inline int					inputDeviceChannels() const;
	inline int					outputDeviceChannels() const;						
};

inline int OSXAudioDevice::Impl::inputDeviceChannels() const
{
	return port[REC].deviceFormat.mChannelsPerFrame;
}

inline int OSXAudioDevice::Impl::outputDeviceChannels() const
{
	return port[PLAY].deviceFormat.mChannelsPerFrame;
}

// Utilities

inline int min(int x, int y) { return (x <= y) ? x : y; }

inline int inAvailable(int filled, int size) {
	return size - filled;
}

#define DEBUG 0

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
	int framesAdvanced = 0;
	Port *port;
#if DEBUG > 0
	printf("OSXAudioDevice: top of runProcess\n");
#endif
	bool keepGoing = true;
	bool callbackCalled = false;
	if (impl->recording) {
		port = &impl->port[REC];
		const int destchans = port->audioBufChannels;
		const int bufLen = port->audioBufFrames * destchans;
		// How many frames are available from HW.
		const int framesToRead = port->deviceBufFrames;
		// How many frames' space are available in our buffer.
		int framesAvail = ::inAvailable(port->audioBufFilled, port->audioBufFrames);

#if DEBUG > 0
		printf("OSXAudioDevice: record section\n");
		printf("framesToRead = %d, framesAvail = %d, Filled = %d\n", framesToRead, framesAvail, port->audioBufFilled);
#endif

		// Run this loop while not enough space to copy audio from HW.
		while (framesAvail < framesToRead && keepGoing) {
			keepGoing = device->runCallback();
			callbackCalled = true;
			framesAvail = ::inAvailable(port->audioBufFilled, port->audioBufFrames);
#if DEBUG > 0
			printf("\tafter run callback, framesAvail = %d\n", framesAvail);
#endif
		}

		if (keepGoing == true) {
#if DEBUG > 1
			printf("\tREC inLoc begins at %d (out of %d)\n", port->inLoc, bufLen);
#endif
			int	framesCopied = 0;
			// Write new audio data into port->audioBuffer.
			//   Treat it as circular buffer.
			while (framesCopied < framesToRead) {
				const int srcchans = impl->inputDeviceChannels();
				register float *src = (float *) inInputData->mBuffers[port->streamIndex].mData;
				register float *dest = port->audioBuffer;
				int inLoc = port->inLoc;
				for (int n = 0; n < framesToRead; ++n) {
					if (inLoc == bufLen)	// wrap
						inLoc = 0;
					for (int ch = 0; ch < destchans; ++ch) {
						if (ch < srcchans)
							dest[inLoc] = src[ch];
						else
							dest[inLoc] = 0.0f;
						++inLoc;
					}
					src += srcchans;
				}
				port->audioBufFilled += framesToRead;
				port->inLoc = inLoc;
				framesCopied = framesToRead;
			}
			framesAdvanced = framesCopied;
#if DEBUG > 0
			printf("\tREC Filled = %d\n", port->audioBufFilled);
#endif
#if DEBUG > 1
			printf("\tREC inLoc ended at %d\n", port->inLoc);
#endif
		}
		else if (!impl->playing) {
			// printf("OSXAudioDevice: run callback returned false -- calling stop callback\n");
			device->stopCallback();
			device->close();
#if DEBUG > 0
			printf("OSXAudioDevice: leaving runProcess\n");
#endif
			return kAudioHardwareNoError;
		}
	}
	if (impl->playing) {
		port = &impl->port[PLAY];
		const int framesToWrite = port->deviceBufFrames;
		const int srcchans = port->audioBufChannels;	// THIS CAN BE LESS THAN 'destchans'
		const int bufLen = port->audioBufFrames * srcchans;
		const int destchans = impl->outputDeviceChannels();
		const int chansToCopy = (srcchans < destchans) ? srcchans : destchans;
		int framesAvail = port->audioBufFilled;
		
#if DEBUG > 0
		printf("OSXAudioDevice: playback section\n");
		printf("framesAvail (Filled) = %d\n", framesAvail);
#endif
		while (framesAvail < framesToWrite && keepGoing) {
			assert(!callbackCalled);
#if DEBUG > 0
			printf("\tframesAvail < needed (%d), so run callback for more\n", framesToWrite);
#endif
			keepGoing = device->runCallback();
			framesAvail = port->audioBufFilled;
#if DEBUG > 0
			printf("\tafter run callback, framesAvail (Filled) = %d\n", framesAvail);
#endif
			if (framesAvail == 0) {
				assert(!keepGoing || device->isPassive());
#if DEBUG > 0
				printf("\tzeroing input buffer and going on\n");
#endif
				memset(port->audioBuffer, 0, bufLen * sizeof(float));
				framesAvail = framesToWrite;
			}
		}
		if (keepGoing == true) {
#if DEBUG > 1
			printf("\tPLAY outLoc begins at %d (out of %d)\n",
				   port->outLoc, bufLen);
#endif
			int framesDone = 0;
			// Audio data has been written into port->audioBuffer during doSendFrames.
			//   Treat it as circular buffer.
			while (framesDone < framesToWrite) {
				register float *src = port->audioBuffer;
				register float *dest = (float *) outOutputData->mBuffers[port->streamIndex].mData;
				int outLoc = port->outLoc;
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
				port->audioBufFilled -= framesToWrite;
				port->outLoc = outLoc;
				framesDone += framesToWrite;
				framesAdvanced = framesDone;
			}
#if DEBUG > 0
			printf("\tPLAY Filled = %d\n", port->audioBufFilled);
#endif
#if DEBUG > 1
			printf("\tPLAY outLoc ended at %d\n", port->outLoc);
#endif
		}
		else {
			// printf("OSXAudioDevice: run callback returned false -- calling stop callback\n");
			device->stopCallback();
			device->close();
		}
	}
	impl->frameCount += framesAdvanced;
#if DEBUG > 0
	printf("OSXAudioDevice: leaving runProcess\n\n");
#endif
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
						kMasterChannel, impl->recording,
						kAudioDevicePropertyDeviceIsRunning,
						&size,
				   		(void *) &isRunning);
		break;
	case kAudioDeviceProcessorOverload:
		fprintf(stderr, "OSXAudioDevice: I/O thread overload!\n");
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

OSXAudioDevice::OSXAudioDevice(const char *desc) : _impl(new Impl)
{
	_impl->deviceID = 0;
	for (int n = REC; n <= PLAY; ++n) {
		_impl->port[n].streamIndex = 0;
		_impl->port[n].streamChannel = 0;
		_impl->port[n].streamDesc = NULL;
		_impl->port[n].deviceBufFrames = 0;
		_impl->port[n].audioBufFrames = 0;
		_impl->port[n].audioBufChannels = 0;
		_impl->port[n].audioBuffer = NULL;
		_impl->port[n].inLoc = _impl->port[n].outLoc = 0;
		_impl->port[n].audioBufFilled = 0;
	}
	char *substr = desc ? strstr(desc, ":") + 1 : NULL;
	// Strip desc of form "OSXHW:inbuf,outbuf" into inbuf and outbuf
	if (substr != NULL) {
		char *outsubstr = NULL;
		int idx1 = 0, idx2 = -1;
		idx1 = strtol(substr, &outsubstr, 0);
		_impl->port[REC].streamIndex = idx1;
		if (outsubstr && strlen(outsubstr) > 0)
			idx2 = strtol(outsubstr + 1, NULL, 0);
		_impl->port[PLAY].streamIndex = (idx2 >= 0) ? idx2 : idx1;

		printf("input streamIndex is %d\n", _impl->port[REC].streamIndex);
		printf("output streamIndex is %d\n", _impl->port[PLAY].streamIndex);

	}
	_impl->bufferSampleFormat = MUS_UNKNOWN;
	_impl->frameCount = 0;
	_impl->paused = false;
	_impl->stopping = false;
	_impl->recording = false;
	_impl->playing = false;
}

OSXAudioDevice::~OSXAudioDevice()
{
	//printf("OSXAudioDevice::~OSXAudioDevice()\n");
	delete [] _impl->port[REC].streamDesc;
	delete [] _impl->port[REC].audioBuffer;
	delete [] _impl->port[PLAY].streamDesc;
	delete [] _impl->port[PLAY].audioBuffer;
	delete _impl;
}

int OSXAudioDevice::openInput()
{
	OSXAudioDevice::Impl *impl = _impl;
	OSXAudioDevice::Impl::Port *port = &impl->port[REC];
	AudioDeviceID devID;
	Boolean isInput = 1;
	Boolean writeable = 0;
	UInt32 size = sizeof(devID);
	OSStatus err = AudioHardwareGetProperty(
						kAudioHardwarePropertyDefaultInputDevice,
						&size,
				   		(void *) &devID);
	if (err != kAudioHardwareNoError || devID == kAudioDeviceUnknown) {
		return error("Cannot find default input device: ", ::errToString(err));
	}
	impl->deviceID = devID;	// If we are opening for output as well, this gets reset.
	// Get the complete stream description set for the input
	err = AudioDeviceGetPropertyInfo(devID, 
									 kMasterChannel,
									 isInput,
									 kAudioDevicePropertyStreamConfiguration,
									 &size,
									 &writeable);
	if (err != kAudioHardwareNoError) {
		return error("Can't get input device property info: ",
					 ::errToString(err));
	}
	port->streamDesc = new AudioBufferList[size/sizeof(AudioBufferList)];
	err = AudioDeviceGetProperty(devID, 
								 kMasterChannel,
								 isInput,
								 kAudioDevicePropertyStreamConfiguration,
								 &size,
								 port->streamDesc);
	if (err != kAudioHardwareNoError) {
		return error("Can't get input device stream configuration: ",
					 ::errToString(err));
	}
	// Check that user's request is a valid stream
	if (port->streamIndex >= port->streamDesc->mNumberBuffers) {
		return error("Invalid input stream index");
	}
	// Brute force: Find first audio channel for desired input stream
	int streamChannel = 1;
	for (int stream = 1; stream < port->streamDesc->mNumberBuffers; ++stream) {
		if (stream == port->streamIndex) {
			port->streamChannel = streamChannel;
			printf("input port streamChannel = %d\n", port->streamChannel);
			break;
		}
		streamChannel += port->streamDesc->mBuffers[stream].mNumberChannels;
	}

	// Get current input format
	size = sizeof(port->deviceFormat);
	err = AudioDeviceGetProperty(devID, 
								  port->streamChannel, isInput,
								  kAudioDevicePropertyStreamFormat, 
								  &size, 
								  &port->deviceFormat);
	if (err != kAudioHardwareNoError) {
		return error("Can't get input device format: ", ::errToString(err));
	}
	// Test and store whether or not audio format property is writable.
	size = sizeof(writeable);
	err = AudioDeviceGetPropertyInfo(devID, 
   									port->streamChannel, isInput,
								    kAudioDevicePropertyStreamFormat,
									&size,
									&writeable);
	if (err != kAudioHardwareNoError) {
		return error("Can't get input device writeable property: ", 
					 	  ::errToString(err));
	}
//	_impl->formatWritable = (writeable != 0);
	return 0;
}

int OSXAudioDevice::openOutput()
{
	OSXAudioDevice::Impl *impl = _impl;
	OSXAudioDevice::Impl::Port *port = &impl->port[PLAY];
	AudioDeviceID devID;
	Boolean isOutput = 0;
	Boolean writeable = 0;
	UInt32 size = sizeof(devID);
	OSStatus err = AudioHardwareGetProperty(
						kAudioHardwarePropertyDefaultOutputDevice,
						&size,
				   		(void *) &devID);
	if (err != kAudioHardwareNoError || devID == kAudioDeviceUnknown) {
		return error("Cannot find default output device: ", ::errToString(err));
	}
	if (_impl->deviceID != 0 && _impl->deviceID != devID)
		printf("Input device ID != output -- ???\n");
	impl->deviceID = devID;
	// Get the complete stream description set for the output
	err = AudioDeviceGetPropertyInfo(devID, 
									 kMasterChannel,
									 isOutput,
									 kAudioDevicePropertyStreamConfiguration,
									 &size,
									 &writeable);
	if (err != kAudioHardwareNoError) {
		return error("Can't get output device property info: ",
					 ::errToString(err));
	}
	port->streamDesc = new AudioBufferList[size/sizeof(AudioBufferList)];
	err = AudioDeviceGetProperty(devID, 
								 kMasterChannel,
								 isOutput,
								 kAudioDevicePropertyStreamConfiguration,
								 &size,
								 port->streamDesc);
	if (err != kAudioHardwareNoError) {
		return error("Can't get output device stream configuration: ",
					 ::errToString(err));
	}
	// Check that user's request is a valid stream
	if (port->streamIndex >= port->streamDesc->mNumberBuffers) {
		return error("Invalid output stream index");
	}
	// Brute force: Find first audio channel for desired output stream
	int streamChannel = 1;
	for (int stream = 1; stream < port->streamDesc->mNumberBuffers; ++stream) {
		if (stream == port->streamIndex) {
			port->streamChannel = streamChannel;
			printf("output port streamChannel = %d\n", port->streamChannel);
			break;
		}
		streamChannel += port->streamDesc->mBuffers[stream].mNumberChannels;
	}

	// Get current output format	
	size = sizeof(port->deviceFormat);
	err = AudioDeviceGetProperty(devID, 
								  port->streamChannel, isOutput,
								  kAudioDevicePropertyStreamFormat, 
								  &size, 
								  &port->deviceFormat);
	if (err != kAudioHardwareNoError) {
		return error("Can't get output device format: ", ::errToString(err));
	}
	// Cache this.
	// Test and store whether or not audio format property is writable.
	size = sizeof(writeable);
	err = AudioDeviceGetPropertyInfo(devID,
   									port->streamChannel, isOutput,
								    kAudioDevicePropertyStreamFormat,
									&size,
									&writeable);
	if (err != kAudioHardwareNoError) {
		return error("Can't get output device writeable property: ", 
					 	  ::errToString(err));
	}
	impl->formatWritable = (writeable != 0);
	return 0;
}

int OSXAudioDevice::doOpen(int mode)
{
	_impl->recording = ((mode & Record) != 0);
	_impl->playing = ((mode & Playback) != 0);
	Boolean isInput = !_impl->playing;
	int status;
	OSStatus err;
	
	if (_impl->recording)
		if ((status = openInput()) < 0)
			return status;
	if (_impl->playing)
		if ((status = openOutput()) < 0)
			return status;
		
	// Register our callback functions with the HAL.
	err = AudioDeviceAddPropertyListener(_impl->deviceID,
										kMasterChannel, isInput,
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
											kMasterChannel, _impl->recording,
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
	OSXAudioDevice::Impl::Port *port;

	_impl->bufferSampleFormat = MUS_GET_FORMAT(fmt);

	// Sanity check, because we do the conversion to float ourselves.
	if (_impl->bufferSampleFormat != MUS_BFLOAT)
		return error("Only float audio buffers supported at this time.");

	if (_impl->formatWritable)
	{
		const int startDir = _impl->recording ? REC : PLAY;
		const int endDir = _impl->playing ? PLAY : REC;
		for (int dir = startDir; dir <= endDir; ++dir) {
			port = &_impl->port[dir];
			// We catch mono input and do the conversion ourselves.  Otherwise we
			// create a buffer equal to the requested channel count.
			port->audioBufChannels = (chans == 1) ? 2 : chans;
			// Default all values to device's defaults (from doOpen()), then set
			// our sample rate.
			AudioStreamBasicDescription requestedFormat = port->deviceFormat;
			requestedFormat.mSampleRate = srate;
			UInt32 size = sizeof(requestedFormat);
			OSStatus err = AudioDeviceSetProperty(_impl->deviceID,
										 NULL,
										 port->streamChannel, dir == REC,
								    	 kAudioDevicePropertyStreamFormat,
										 size,
										 (void *)&requestedFormat);
			if (err != kAudioHardwareNoError) {
				return error("Can't set audio device format: ", ::errToString(err));
			}
			// Now retrieve settings to see what we got (IS THIS NECESSARY?)
			size = sizeof(port->deviceFormat);
			err = AudioDeviceGetProperty(_impl->deviceID, 
										  port->streamChannel, dir == REC,
										  kAudioDevicePropertyStreamFormat, 
										  &size, 
										  &port->deviceFormat);
			if (err != kAudioHardwareNoError) {
				return error("Can't retrieve audio device format: ",
							 ::errToString(err));
			}
			else if (port->deviceFormat.mSampleRate != srate) {
				return error("This sampling rate not supported.");
			}
#if DEBUG > 0
			printf("OSX %s HW %d channel, %s\n",
				   dir == REC ? "input" : "output",
				   port->deviceFormat.mChannelsPerFrame,
					port->deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsNonInterleaved ? "non-interleaved" : "interleaved");
#endif
		}
	}
	// If format was not writable, see if our request matches defaults.
	else if (_impl->port[_impl->playing ? PLAY : REC].deviceFormat.mSampleRate != srate) {
		return error("Audio sample rate not configurable on this device");
	}
	
	int deviceFormat = MUS_BFLOAT | MUS_NORMALIZED;
	// We set the device format based upon whether we are recording or playing.
	int portIndex = _impl->playing ? PLAY : REC;
	port = &_impl->port[portIndex];

#ifdef WHEN_NONINTERLEAVED_IS_FINISHED
	// Set the device format based upon settings.  This will be used for format conversion.
	if ((port->deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsNonInterleaved) != 0) {
#if DEBUG > 0
		printf("OSX HW is %d channel, non-interleaved\n", port->deviceFormat.mChannelsPerFrame);
#endif
		deviceFormat |= MUS_NON_INTERLEAVED;
	}
	else {
#if DEBUG > 0
		printf("OSX HW is %d channel, interleaved\n", port->deviceFormat.mChannelsPerFrame);
#endif
		deviceFormat |= MUS_INTERLEAVED;
	}
#else	/* WHEN_NONINTERLEAVED_IS_FINISHED */
	// Temporarily, we report back the format of the circular buffers
	// that I use as my intermediate buffers.  This is different from
	// the AudioDeviceImpl's conversion buffer, and involves a second
	// buffer copy, for the time being.

#if DEBUG > 0
	printf("OSX HW is %d channel, uses temp interleaved buffer\n", _impl->port[portIndex].deviceFormat.mChannelsPerFrame);
#endif
	deviceFormat |= MUS_INTERLEAVED;
#endif	/* WHEN_NONINTERLEAVED_IS_FINISHED */

	setDeviceParams(deviceFormat,
					port->deviceFormat.mChannelsPerFrame,
					port->deviceFormat.mSampleRate);
	return 0;
}

int OSXAudioDevice::doSetQueueSize(int *pWriteSize, int *pCount)
{
	Boolean writeable;
	UInt32 size = sizeof(writeable);
	OSStatus err;
	int reqQueueFrames = *pWriteSize;
	unsigned int deviceBufferBytes = 0;

	const int startDir = _impl->recording ? REC : PLAY;
	const int endDir = _impl->playing ? PLAY : REC;
	for (int dir = startDir; dir <= endDir; ++dir) {
		Impl::Port *port = &_impl->port[dir];
#if DEBUG > 0
		printf("========== CONFIGURING %s ==========\n", dir == REC ? "INPUT" : "OUTPUT");
#endif
		err = AudioDeviceGetPropertyInfo(_impl->deviceID, 
   									port->streamChannel, dir == REC,
								    kAudioDevicePropertyBufferSize, 
									&size,
									&writeable);
		if (err != kAudioHardwareNoError) {
			return error("Can't get audio device property");
		}
		// Audio buffer is always floating point.  Attempt to set size in bytes.
		// Loop until request is accepted, halving value each time.
		unsigned int reqBufBytes = sizeof(float) * port->deviceFormat.mChannelsPerFrame * reqQueueFrames;
		if (writeable) {
			size = sizeof(reqBufBytes);
			while ( (err = AudioDeviceSetProperty(_impl->deviceID,
											 NULL,
											 port->streamChannel, dir == REC,
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
		size = sizeof(deviceBufferBytes);
		err = AudioDeviceGetProperty(_impl->deviceID,
									 port->streamChannel, dir == REC,
									 kAudioDevicePropertyBufferSize,
									 &size,
									 &deviceBufferBytes);
		if (err != kAudioHardwareNoError) {
			return error("Can't get audio device buffer size");
		}

		int deviceFrames = deviceBufferBytes / (sizeof(float) * port->deviceFormat.mChannelsPerFrame);
#if DEBUG > 0
		printf("OSX device buffer size is %d frames, user req was %d frames\n",
				deviceFrames, reqQueueFrames);
#endif
		// We allocate the circular buffers to be the max(2_times_HW, user_req).
		if (deviceFrames * 2 > reqQueueFrames) {
			port->audioBufFrames = 2 * deviceFrames;
		}
		else {
			// Make audio buffer a multiple of hw buffer
			port->audioBufFrames = reqQueueFrames - (reqQueueFrames % deviceFrames);
		}
		// Notify caller of any change.
		*pWriteSize = port->audioBufFrames;

		port->deviceBufFrames = deviceBufferBytes / (port->deviceFormat.mChannelsPerFrame * sizeof(float));

#if DEBUG > 0
		printf("%s device bufsize: %d bytes (%d frames). circ buffer %d frames\n",
				dir == REC ? "input" : "output", deviceBufferBytes,
				port->deviceBufFrames, port->audioBufFrames);
#endif
		int buflen = port->audioBufFrames * port->audioBufChannels;
		delete [] port->audioBuffer;
		port->audioBuffer = new float[buflen];
		if (port->audioBuffer == NULL)
			return error("Memory allocation failure for OSXAudioDevice buffer!");
		memset(port->audioBuffer, 0, sizeof(float) * buflen);
		// NEW: Set play buffer as filled with zeros.
		if (dir == PLAY)
			port->audioBufFilled = port->audioBufFrames;
		port->inLoc = 0;
		port->outLoc = 0;
	}
	return 0;
}

int	OSXAudioDevice::doGetFrames(void *frameBuffer, int frameCount)
{
	const int frameChans = getFrameChannels();
	Impl::Port *port = &_impl->port[REC];
	const int bufChannels = port->audioBufChannels;
	const int bufLen = port->audioBufFrames * bufChannels;
	int outLoc = port->outLoc;
#if DEBUG > 0
	printf("OSXAudioDevice::doGetFrames: frameCount = %d REC filled = %d\n", frameCount, port->audioBufFilled);
#endif
#if DEBUG > 1
	printf("\tREC outLoc begins at %d (out of %d)\n", outLoc, bufLen);
#endif
	assert(frameCount <= port->audioBufFilled);

	switch (frameChans) {
	case 1:
		if (bufChannels == 2) {
#if DEBUG > 1
			printf("Copying stereo buf into mono user frame\n");
#endif
			const float scale = 0.707;
			float *from = port->audioBuffer;
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
		if (bufChannels == frameChans) {
#if DEBUG > 1
			printf("Copying buf into user frame\n");
#endif
			float *from = port->audioBuffer;
			float *outbuf = (float *) frameBuffer;
			// Write all channels of each frame from circ. buffer into output frame.
			for (int out=0; out < frameCount; ++out) {
				if (outLoc >= bufLen)
					outLoc -= bufLen;	// wrap
				for (int ch = 0; ch < frameChans; ++ch) {
					outbuf[ch] = from[outLoc++];	
				}
				outbuf += frameChans;
			}
		}
		else
			return error("Channel count conversion not supported for record");
		break;
	}
	port->outLoc = outLoc;
	port->audioBufFilled -= frameCount;
#if DEBUG > 0
	printf("\tREC Filled now %d\n", port->audioBufFilled);
#endif
#if DEBUG > 1
	printf("\tREC outLoc ended at %d. Returning frameCount = %d\n", outLoc, frameCount);
#endif
	return frameCount;
}

int	OSXAudioDevice::doSendFrames(void *frameBuffer, int frameCount)
{
	const int frameChans = getFrameChannels();
	Impl::Port *port = &_impl->port[PLAY];
	float *outbuf = port->audioBuffer;
	const int bufChannels = port->audioBufChannels;
	const int bufLen = port->audioBufFrames * port->audioBufChannels;
	int inLoc = port->inLoc;
#if DEBUG > 0
	printf("OSXAudioDevice::doSendFrames: frameCount = %d, PLAY filled = %d\n", frameCount, port->audioBufFilled);
#endif
#if DEBUG > 1
	printf("\tPLAY inLoc begins at %d (out of %d)\n", inLoc, bufLen);
#endif
	switch (frameChans) {
	case 1:		// Mono input converted to stereo circ. buffer;  HW 2-N channels.
		if (bufChannels == 2) {
#if DEBUG > 1
			printf("Copying mono user frame into stereo buf\n");
#endif
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
		if (bufChannels == frameChans) {
#if DEBUG > 1
			printf("Copying user frame into buf\n");
#endif
			float *from = (float *) frameBuffer;
			// Write all channels of each frame from frame into circular buf.
			for (int in=0; in < frameCount; ++in) {
				if (inLoc >= bufLen)
					inLoc -= bufLen;	// wrap
				for (int ch = 0; ch < frameChans; ++ch) {
					outbuf[inLoc+ch] = (float) from[ch];	
				}
				from += frameChans;
				inLoc += bufChannels;
			}
		}
		else
			return error("Channel count conversion not supported for playback");
		break;
	}
	port->audioBufFilled += frameCount;
	port->inLoc = inLoc;
#if DEBUG > 0
	printf("\tPLAY Filled now %d\n", port->audioBufFilled);
#endif
#if DEBUG > 1
	printf("\tPLAY inLoc ended at %d. Returning frameCount = %d\n", inLoc, frameCount);
#endif
	return frameCount;
}

int OSXAudioDevice::doGetFrameCount() const
{
	return _impl->frameCount;
}

bool OSXAudioDevice::recognize(const char *desc)
{
	return desc == NULL | strncmp(desc, "OSXHW", 5) == 0;
}

AudioDevice *OSXAudioDevice::create(const char *inputDesc, const char *outputDesc, int mode)
{
	return new OSXAudioDevice(inputDesc ? inputDesc : outputDesc);
}

#endif	// MACOSX
