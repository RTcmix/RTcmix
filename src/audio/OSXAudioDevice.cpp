// OSXAudioDevice.cpp

#if defined(MACOSX)

#include "OSXAudioDevice.h"
#include <CoreAudio/CoreAudio.h>
#include <mach/mach.h>
#include <mach/policy.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sndlibsupport.h>	// RTcmix header

#define DEBUG 0

#if DEBUG > 0
#define ENTER(fun) printf("Entering " #fun "\n");
#else
#define ENTER(fun)
#endif

static const char *errToString(OSStatus err);
static OSStatus getDeviceList(AudioDeviceID **devList, int *devCount);
static AudioDeviceID findDeviceID(const char *devName, AudioDeviceID *devList,
								  int devCount, Boolean isInput);

static const int REC = 0, PLAY = 1;
static const int kMasterChannel = 0;

struct OSXAudioDevice::Impl {
	AudioDeviceID			*deviceIDs;				// Queried on system.
	int						deviceCount;			// Queried on system.
	char 					*deviceName;			// Passed in by user.
	AudioDeviceID			deviceID;
	struct Port {
		int						streamIndex;		// Which stream
		int						streamCount;		// How many streams open
		int						streamChannel;		// 1st chan of first stream
		AudioBufferList			*streamDesc;		// Properties for all strams
		AudioStreamBasicDescription deviceFormat;		// format
		unsigned int 			deviceBufFrames;	// hw buf length
		float					*audioBuffer;		// circ. buffer
		int						audioBufFrames;		// length of audioBuffers
		int						audioBufChannels;	// channels in audioBuffers
		int						virtualChannels;	// what is reported publically (may vary based on mono-stereo)
		int						inLoc, outLoc;		// circ. buffer indices
		int						audioBufFilled;		// audioBuffer samples available
		typedef int (*FrameFunction)(struct Port *, void *, int, int);
		FrameFunction			frameFn;
		static int				interleavedGetFrames(struct Port *, void *, int, int);
		static int				interleavedSendFrames(struct Port *, void *, int, int);
		static int				noninterleavedGetFrames(struct Port *, void *, int, int);
		static int				noninterleavedSendFrames(struct Port *, void *, int, int);
	} 						port[2];
	
	int						bufferSampleFormat;
	int						frameCount;
	bool					formatWritable;		// true if device allows changes to fmt
	bool					paused;
	bool					stopping;
	bool					recording;				// Used by OSX code
	bool					playing;				// Used by OSX code
	static OSStatus			runProcess(
								AudioDeviceID			inDevice,
								const AudioTimeStamp	*inNow,
								const AudioBufferList	*inInputData,
								const AudioTimeStamp	*inInputTime,
								AudioBufferList			*outOutputData,
								const AudioTimeStamp	*inOutputTime,
						  		void					*object);
	static OSStatus			listenerProcess(
								AudioDeviceID inDevice,
								UInt32 inChannel,
								Boolean isInput,
								AudioDevicePropertyID inPropertyID,
								void *object);

	inline int				inputDeviceChannels() const;
	inline int				outputDeviceChannels() const;						
};

// I/O Functions

int
OSXAudioDevice::Impl::Port::interleavedGetFrames(struct Port *port, 
                                                 void *frameBuffer,
                                                 int frameCount, int frameChans)
{
	const int bufChannels = port->audioBufChannels;
	const int bufLen = port->audioBufFrames * bufChannels;
	int bufLoc = port->outLoc;
#if DEBUG > 0
	printf("OSXAudioDevice::doGetFrames: frameCount = %d REC filled = %d\n", frameCount, port->audioBufFilled);
#endif
#if DEBUG > 1
	printf("\tREC bufLoc begins at %d (out of %d)\n", bufLoc, bufLen);
#endif
	assert(frameCount <= port->audioBufFilled);

	if (frameChans == 1 && bufChannels == 2) {
#if DEBUG > 0
		printf("Copying stereo buf into mono user frame\n");
#endif
		const float scale = 0.707;
		float *buf = port->audioBuffer;
		float *frame = (float *) frameBuffer;
		// Combine stereo channels from circ. buffer into mono output frame.
		for (int out=0; out < frameCount; ++out) {
			if (bufLoc >= bufLen)	// wrap
				bufLoc -= bufLen;
			frame[out] = (buf[bufLoc] + buf[bufLoc+1]) * scale;	
			bufLoc += 2;
		}
	}
	else {
#if DEBUG > 0
		printf("Copying %d-channel interleaved buf into %d-channel user frame\n", bufChannels, frameChans);
#endif
		float *buf = port->audioBuffer;
		float *frame = (float *) frameBuffer;
		// For each frame, copy channels from our interleaved buffer up to the output frame's max. 
		for (int out=0; out < frameCount; ++out) {
			int ch;
			if (bufLoc >= bufLen)
				bufLoc -= bufLen;	// wrap
			for (ch = 0; ch < bufChannels; ++ch) {
				if (ch < frameChans) {
					frame[ch] = buf[bufLoc];	
				}
				++bufLoc;
			}
			// Zero any extra frame channels
			for (; ch < frameChans; ++ch)
				frame[ch] = 0.0f;
				
			frame += frameChans;
		}
	}
	port->outLoc = bufLoc;
	port->audioBufFilled -= frameCount;
#if DEBUG > 1
	printf("\tREC Filled now %d\n", port->audioBufFilled);
#endif
#if DEBUG > 1
	printf("\tREC bufLoc ended at %d. Returning frameCount = %d\n", bufLoc, frameCount);
#endif
	return frameCount;
}

int
OSXAudioDevice::Impl::Port::interleavedSendFrames(struct Port *port,
                                                  void *frameBuffer,
                                                  int frameCount, int frameChans)
{
	float *buf = port->audioBuffer;
	const int bufChannels = port->audioBufChannels;
	// Length in samples, not frames.
	const int bufLen = port->audioBufFrames * bufChannels;
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
#if DEBUG > 0
			printf("Copying mono user frame into our interleaved stereo buf\n");
#endif
			float *frame = (float *) frameBuffer;
			for (int in=0; in < frameCount; ++in) {
				if (inLoc >= bufLen)	// wrap
					inLoc -= bufLen;
				// Write single channel from frame into both chans of circular buf.
				buf[inLoc+1] = buf[inLoc] = *frame++;	
				inLoc += 2;
			}
		}
		else {
#if DEBUG > 0
			printf("Copying mono user frame into first two chans of our interleaved buf\n");
#endif
			float *frame = (float *) frameBuffer;
			for (int in=0; in < frameCount; ++in) {
				if (inLoc >= bufLen) // wrap
					inLoc -= bufLen;
				// Write single channel from frame into first two chans of circular buf.
				buf[inLoc+1] = buf[inLoc] = (float)(*frame);
				for (int ch = 2; ch < bufChannels; ++ch) {
					buf[inLoc+ch] = 0.0f;
				}
				frame += frameChans;
				inLoc += bufChannels;
			}
		}
		break;

	default:	// 2-N chan input to 2-N channel circ. buffer; HW 2-N chans.
		{
#if DEBUG > 0
			printf("Copying %d-channel user frame into our %d-channel interleaved buf\n",
				   frameChans, bufChannels);
#endif
			float *frame = (float *) frameBuffer;
			// Write all channels of each frame from frame into circular buf.
			for (int in=0; in < frameCount; ++in) {
				if (inLoc >= bufLen)
					inLoc -= bufLen;	// wrap
				for (int ch = 0; ch < bufChannels; ++ch) {
					if (ch < frameChans)
						buf[inLoc+ch] = (float) frame[ch];
					else {
						buf[inLoc+ch] = 0.0f;
					}
				}
				frame += frameChans;
				inLoc += bufChannels;
			}
		}
		break;
	}
	port->audioBufFilled += frameCount;
	port->inLoc = inLoc;
#if DEBUG > 1
	printf("\tPLAY Filled now %d\n", port->audioBufFilled);
#endif
#if DEBUG > 1
	printf("\tPLAY inLoc ended at %d. Returning frameCount = %d\n", inLoc, frameCount);
#endif
	return frameCount;
}

int
OSXAudioDevice::Impl::Port::noninterleavedGetFrames(struct Port *port, 
                                                    void *frameBuffer,
                                                    int frameCount, int frameChans)
{
	const int streamCount = port->streamCount;	// for non-interleaved, this is the internal channel count
	const int bufFrames = port->audioBufFrames;
	int bufLoc = 0;
	float **fFrameBuffer = (float **) frameBuffer;		// non-interleaved
#if DEBUG > 0
	printf("OSXAudioDevice::doGetFrames: frameCount = %d REC filled = %d\n", frameCount, port->audioBufFilled);
#endif
	assert(frameCount <= port->audioBufFilled);

	const int streamsToCopy = frameChans < streamCount ? frameChans : streamCount;
#if DEBUG > 0
	printf("Copying %d non-interleaved internal bufs into %d-channel user frame\n",
		   streamsToCopy, frameChans);
#endif
	int stream;
	for (stream = 0; stream < streamsToCopy; ++stream) {
		// Offset into serially-ordered, multi-channel non-interleaved buf.
		register float *buf = &port->audioBuffer[stream * bufFrames];
		float *frame = fFrameBuffer[stream];
		bufLoc = port->outLoc;
#if DEBUG > 1
		printf("\tstream %d: raw offset into mono internal buffer: %d (%d * %d)\n", stream, buf - &port->audioBuffer[0], stream, bufFrames);
		printf("\tread internal (already-offset) buf starting at outLoc %d\n", bufLoc);
#endif
		// Write each monaural frame from circ. buffer into a non-interleaved output frame.
		for (int out=0; out < frameCount; ++out) {
			if (bufLoc >= bufFrames)
				bufLoc -= bufFrames;	// wrap
			frame[out] = buf[bufLoc++];	
		}
	}
	// Zero out any remaining frame channels
	for (; stream < frameChans; ++stream) {
#if DEBUG > 0
		printf("Zeroing user frame channel %d\n", stream);
#endif
		memset(fFrameBuffer[stream], 0, frameCount * sizeof(float));
	}
	port->outLoc = bufLoc;
	port->audioBufFilled -= frameCount;
#if DEBUG > 1
	printf("\tREC Filled now %d\n", port->audioBufFilled);
#endif
#if DEBUG > 1
	printf("\tREC bufLoc ended at %d. Returning frameCount = %d\n", bufLoc, frameCount);
#endif
	return frameCount;
}

int
OSXAudioDevice::Impl::Port::noninterleavedSendFrames(struct Port *port,
                                                     void *frameBuffer,
                                                     int frameCount, 
													 int frameChans)
{
	float **fFrameBuffer = (float **) frameBuffer;		// non-interleaved
	const int streamCount = port->streamCount;	// for non-interleaved, this is the internal channel count
	const int bufFrames = port->audioBufFrames;
	int inLoc = 0;
#if DEBUG > 0
	printf("OSXAudioDevice::doSendFrames: frameCount = %d, PLAY filled = %d\n", frameCount, port->audioBufFilled);
#endif
#if DEBUG > 0
	printf("Copying %d channel user frame into %d non-interleaved internal buf channels\n", frameChans, streamCount);
#endif
	for (int stream = 0; stream < streamCount; ++stream) {
		inLoc = port->inLoc;
		float *frame = fFrameBuffer[stream];
		// Offset into serially-ordered, multi-channel non-interleaved buf.
		float *buf = &port->audioBuffer[stream * bufFrames];
		if (stream < frameChans) {
			// Write each non-interleaved input frame into circular buf.
			for (int in=0; in < frameCount; ++in) {
				if (inLoc >= bufFrames)
					inLoc -= bufFrames;	// wrap
				buf[inLoc++] = frame[in];	
			}
		}
		else {
#if DEBUG > 0
			printf("Zeroing internal buf channel %d\n", stream);
#endif
			for (int in=0; in < frameCount; ++in) {
				if (inLoc >= bufFrames)
					inLoc -= bufFrames;	// wrap
				buf[inLoc++] = 0.0f;	
			}
		}
	}
	port->audioBufFilled += frameCount;
	port->inLoc = inLoc;
#if DEBUG > 1
	printf("\tPLAY Filled now %d\n", port->audioBufFilled);
#endif
#if DEBUG > 1
	printf("\tPLAY inLoc ended at %d. Returning frameCount = %d\n", inLoc, frameCount);
#endif
	return frameCount;
}

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
		// Length in samples, not frames.
		const int bufLen = port->audioBufFrames * destchans;
		// How many frames are available from HW.
		const int framesToRead = port->deviceBufFrames;
		// How many frames' space are available in our buffer.
		int framesAvail = ::inAvailable(port->audioBufFilled, port->audioBufFrames);

#if DEBUG > 0
		printf("OSXAudioDevice: record section (in buffer %d)\n", port->streamIndex);
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
			// Write input HW's audio data into port->audioBuffer.
			//   Treat it as circular buffer.  Channel counts always match here.
			while (framesCopied < framesToRead) {
				const int srcchans = impl->inputDeviceChannels();
				int inLoc = port->inLoc;
				for (int stream = 0; stream < port->streamCount; ++stream) {
					inLoc = port->inLoc;
					const int strIdx = stream + port->streamIndex;
					register float *src = (float *) inInputData->mBuffers[strIdx].mData;
					register float *dest = &port->audioBuffer[stream * port->audioBufFrames];
#if DEBUG > 1
					printf("\tstream %d: copying from HW buffer %d into internal buf at raw offset %d (%d * %d)\n",
						   stream, strIdx, dest - &port->audioBuffer[0], stream, port->audioBufFrames);
					printf("\t incrementing HW buffer pointer by %d for each frame\n", srcchans);
#endif
					for (int n = 0; n < framesToRead; ++n) {
						if (inLoc == bufLen)	// wrap
							inLoc = 0;
						for (int ch = 0; ch < destchans; ++ch) {
							dest[inLoc++] = src[ch];
						}
						src += srcchans;
					}
				}
				port->inLoc = inLoc;
				port->audioBufFilled += framesToRead;
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
		const int destchans = impl->outputDeviceChannels();
		const int srcchans = port->audioBufChannels;
		const int bufLen = port->audioBufFrames * srcchans;
		int framesAvail = port->audioBufFilled;
		
#if DEBUG > 0
		printf("OSXAudioDevice: playback section (out buffer %d)\n", port->streamIndex);
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
			if (framesAvail <= 0) {
				assert(!keepGoing || device->isPassive());
#if DEBUG > 0
				printf("\tzeroing input buffer and going on\n");
#endif
				memset(port->audioBuffer, 0, bufLen * port->streamCount * sizeof(float));
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
			// Copy that audio into the output HW's buffer.  Channel counts always match.
			while (framesDone < framesToWrite) {
				int bufLoc = port->outLoc;
#if DEBUG > 0
				printf("\tLooping for %d (%d-channel) stream%s\n",
					   port->streamCount, destchans, port->streamCount > 1 ? "s" : "");
#endif
				for (int stream = 0; stream < port->streamCount; ++stream) {
					const int strIdx = stream + port->streamIndex;
					register float *src = &port->audioBuffer[stream * port->audioBufFrames];
					register float *dest = (float *) outOutputData->mBuffers[strIdx].mData;
					bufLoc = port->outLoc;
					for (int n = 0; n < framesToWrite; ++n) {
						if (bufLoc == bufLen)	// wrap
							bufLoc = 0;
						for (int ch = 0; ch < destchans; ++ch) {
							dest[ch] = src[bufLoc++];
						}
						dest += destchans;
					}
				}
				port->audioBufFilled -= framesToWrite;
				port->outLoc = bufLoc;
				framesDone += framesToWrite;
				framesAdvanced = framesDone;
			}
#if DEBUG > 0
			printf("\tPLAY Filled = %d\n", port->audioBufFilled);
#endif
#if DEBUG > 1
			printf("\tPLAY bufLoc ended at %d\n", port->outLoc);
#endif
		}
		else {
#if DEBUG > 0
			printf("OSXAudioDevice: run callback returned false -- calling stop callback\n");
#endif
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
	_impl->deviceIDs = NULL;
	_impl->deviceID = 0;
	_impl->deviceName = NULL;
	for (int n = REC; n <= PLAY; ++n) {
		_impl->port[n].streamIndex = 0;
		_impl->port[n].streamCount = 1;		// Default;  reset if necessary
		_impl->port[n].streamChannel = 0;
		_impl->port[n].streamDesc = NULL;
		_impl->port[n].deviceBufFrames = 0;
		_impl->port[n].audioBufFrames = 0;
		_impl->port[n].audioBufChannels = 0;
		_impl->port[n].virtualChannels = 0;
		_impl->port[n].audioBuffer = NULL;
		_impl->port[n].inLoc = _impl->port[n].outLoc = 0;
		_impl->port[n].audioBufFilled = 0;
		_impl->port[n].frameFn = NULL;
	}
	::getDeviceList(&_impl->deviceIDs, &_impl->deviceCount);
	
	if (desc != NULL) {
		char *substr = strchr(desc, ':');
		if (substr == NULL) {
			// Descriptor is just the device name
			_impl->deviceName = new char[strlen(desc) + 1];
			strcpy(_impl->deviceName, desc);
		}
		else {
			// Extract device name
			size_t nameLen = (size_t) substr - (size_t) desc;
			_impl->deviceName = new char[nameLen + 1];
			strncpy(_impl->deviceName, desc, nameLen);
			_impl->deviceName[nameLen] = '\0';
			++substr;	// skip ':'
         	// Extract input and output stream selecters
			char *insubstr = NULL, *outsubstr = NULL;
            if ((outsubstr = strchr(substr, ',')) != NULL) {
               ++outsubstr;   // skip ','
               insubstr = substr;
               insubstr[(size_t) outsubstr - (size_t) insubstr - 1] = '\0';
            }
            else {
               insubstr = outsubstr = substr;
            }
            // Now parse stream selecters
            const char *selecters[2] = { insubstr, outsubstr };
            for (int dir = REC; dir <= PLAY; ++dir) {
			   if (selecters[dir] == NULL) {
				  // Do nothing;  use defaults.
			   }
               else if (strchr(selecters[dir], '-') == NULL) {
                  // Parse non-range selecter (single digit)
                  _impl->port[dir].streamIndex = (int) strtol(selecters[dir], NULL, 0);
               }
               else {
                  // Parse selecter of form "X-Y"
                  int idx0, idx1;
                  int found = sscanf(selecters[dir], "%d-%d", &idx0, &idx1);
                  if (found == 2) {
                     _impl->port[dir].streamIndex = idx0;
                     _impl->port[dir].streamCount = idx1 - idx0 + 1;
                  }
                  else {
                     fprintf(stderr, "Could not parse OSX device descriptor \"%s\"\n", desc);
                     break;      
                  }
               }
            }
#if DEBUG > 0
			printf("input streamIndex = %d, requested streamCount = %d\n",
                _impl->port[REC].streamIndex, _impl->port[REC].streamCount);
			printf("output streamIndex = %d, requested streamCount = %d\n",
                _impl->port[PLAY].streamIndex, _impl->port[PLAY].streamCount);
#endif	
        }
		// Treat old-stye device name as "default" (handled below).
		if (!strcmp(_impl->deviceName, "OSXHW")) {
			delete [] _impl->deviceName;
			_impl->deviceName = NULL;
		}
	}
	if (_impl->deviceName == NULL) {
		_impl->deviceName = new char[strlen("default") + 1];
		strcpy(_impl->deviceName, "default");
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
	close();
	delete [] _impl->port[REC].streamDesc;
	delete [] _impl->port[REC].audioBuffer;
	delete [] _impl->port[PLAY].streamDesc;
	delete [] _impl->port[PLAY].audioBuffer;
	delete [] _impl->deviceIDs;
	delete [] _impl->deviceName;
	delete _impl;
}

int OSXAudioDevice::openInput()
{
	ENTER(OSXAudioDevice::openInput());
	OSXAudioDevice::Impl *impl = _impl;
	OSXAudioDevice::Impl::Port *port = &impl->port[REC];
	AudioDeviceID devID = 0;
	Boolean isInput = 1;
	Boolean writeable = 0;
	UInt32 size = sizeof(devID);
	OSStatus err;
	devID = ::findDeviceID(impl->deviceName, impl->deviceIDs, 
	                       impl->deviceCount, isInput);

	if (devID == 0) {
		char msg[64];
		snprintf(msg, 64, "No matching input device found for '%s'\n", impl->deviceName);
		return error(msg);
	}
	
	impl->deviceID = devID;
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
	port->streamDesc = new AudioBufferList[size / (sizeof(AudioBufferList) - sizeof(UInt32))];
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
	// Check that user's request is a valid stream set
	if (port->streamIndex + port->streamCount > (int) port->streamDesc->mNumberBuffers) {
		return error("Invalid input stream set");
	}
	// Brute force: Find first audio channel for desired input stream
	int streamChannel = 1;
	const int streamCount = port->streamDesc->mNumberBuffers;
	if (streamCount > 1) {
		for (int stream = 0; stream < streamCount; ++stream) {
			if (stream == port->streamIndex) {
				port->streamChannel = streamChannel;
#if DEBUG > 0
				printf("input port streamChannel = %d\n", port->streamChannel);
#endif
				break;
			}
			streamChannel += port->streamDesc->mBuffers[stream].mNumberChannels;
		}
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
	ENTER(OSXAudioDevice::openOutput());
	OSXAudioDevice::Impl *impl = _impl;
	OSXAudioDevice::Impl::Port *port = &impl->port[PLAY];
	AudioDeviceID devID = 0;
	Boolean isOutput = 0;
	Boolean writeable = 0;
	UInt32 size = sizeof(devID);
	OSStatus err;
	if (impl->deviceID == 0) {
		devID = ::findDeviceID(impl->deviceName, impl->deviceIDs, 
							   impl->deviceCount, isOutput);
		if (devID == 0) {
			char msg[64];
			snprintf(msg, 64, "No matching output device found for '%s'\n", impl->deviceName);
			return error(msg);
		}
		impl->deviceID = devID;
	}
	else
		devID = impl->deviceID;
	
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
	port->streamDesc = new AudioBufferList[size / (sizeof(AudioBufferList) - sizeof(UInt32))];
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
	// Check that user's request is a valid stream set
	if (port->streamIndex + port->streamCount > (int) port->streamDesc->mNumberBuffers) {
		return error("Invalid output stream set");
	}
	// Brute force: Find first audio channel for desired output stream
	int streamChannel = 1;
	const int streamCount = port->streamDesc->mNumberBuffers;
	if (streamCount > 1) {
		for (int stream = 0; stream < streamCount; ++stream) {
			if (stream == port->streamIndex) {
				port->streamChannel = streamChannel;
#if DEBUG > 0
				printf("output port streamChannel = %d\n", port->streamChannel);
#endif
				break;
			}
			streamChannel += port->streamDesc->mBuffers[stream].mNumberChannels;
		}
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
#if DEBUG > 0
	printf("Output device stream %d (dev channel %d): %u channels\n",
			port->streamIndex, port->streamChannel, 
			port->deviceFormat.mChannelsPerFrame);
#endif
	impl->formatWritable = (writeable != 0);
	return 0;
}

int OSXAudioDevice::doOpen(int mode)
{
	if (mode & Passive) {
		return error("OSX Audio devices do not support passive device mode");
	}
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
	ENTER(OSXAudioDevice::doClose());
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
	ENTER(OSXAudioDevice::doStart());
	_impl->stopping = false;
	OSStatus err = AudioDeviceStart(_impl->deviceID, _impl->runProcess);
	int status = (err == kAudioHardwareNoError) ? 0 : -1;
	if (status == -1)
		error("OSXAudioDevice::doStart: ", errToString(err));
	return status;
}

int OSXAudioDevice::doPause(bool pause)
{
	_impl->paused = pause;
	return error("OSXAudioDevice: pause not yet implemented");
}

int OSXAudioDevice::doStop()
{
	ENTER(OSXAudioDevice::doStop());
	_impl->stopping = true;	// avoids multiple stop notifications
	OSStatus err = AudioDeviceStop(_impl->deviceID, _impl->runProcess);
	int status = (err == kAudioHardwareNoError) ? 0 : -1;
	if (status == -1)
		error("OSXAudioDevice::doStop: ", errToString(err));
	return status;
}

int OSXAudioDevice::doSetFormat(int fmt, int chans, double srate)
{
	ENTER(OSXAudioDevice::doSetFormat());
	OSXAudioDevice::Impl::Port *port;

	_impl->bufferSampleFormat = MUS_GET_FORMAT(fmt);

	// Sanity check, because we do the conversion to float ourselves.
	if (_impl->bufferSampleFormat != MUS_BFLOAT && _impl->bufferSampleFormat != MUS_LFLOAT)
		return error("Only float audio buffers supported at this time.");

	const int startDir = _impl->recording ? REC : PLAY;
	const int endDir = _impl->playing ? PLAY : REC;
	
	for (int dir = startDir; dir <= endDir; ++dir) {
		port = &_impl->port[dir];
		AudioStreamBasicDescription requestedFormat = port->deviceFormat;
		UInt32 hwChannels = port->deviceFormat.mChannelsPerFrame;
		requestedFormat.mSampleRate = srate;
		UInt32 size = sizeof(requestedFormat);
		OSStatus err;
		if (_impl->formatWritable)
		{
			// Default all values to device's defaults (from doOpen()), 
			// then set our sample rate.
			OSStatus err = AudioDeviceSetProperty(_impl->deviceID,
										 NULL,
										 port->streamChannel, dir == REC,
								    	 kAudioDevicePropertyStreamFormat,
										 size,
										 (void *)&requestedFormat);
			if (err != kAudioHardwareNoError) {
				return error("Can't set audio device format: ", 
							 ::errToString(err));
			}
		}
		// Now retrieve settings to see what we got, and compare with request.
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
			return error("Sampling rate not supported or cannot be set.");
		}
		// Always report our channel count to be what we were configured for.  We do all channel
		// matching ourselves during doGetFrames and doSendFrames.
		port->virtualChannels = chans;

		// We create a buffer equal to the internal HW channel count.
		if (port->streamCount == 1) {
			port->audioBufChannels = hwChannels;
		}
		else {
            if (hwChannels > 1) {
               return error("Cannot open multiple multi-channel streams");
            }
			port->audioBufChannels = 1;
		}
#if DEBUG > 0
		printf("OSX %s HW (stream index %d, stream channel %d): %u %s channel(s)\n",
			   dir == REC ? "input" : "output",
			   port->streamIndex, port->streamChannel,
			   hwChannels,
			   port->deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsNonInterleaved ? "non-interleaved" : "interleaved");
		printf("\t reporting %d channels publically\n", port->virtualChannels);
#endif
	}
	
	int deviceFormat = NATIVE_FLOAT_FMT | MUS_NORMALIZED;
	// We set the device format based upon whether we are recording or playing, with playing taking priority.
	int portIndex = _impl->playing ? PLAY : REC;
	port = &_impl->port[portIndex];

	// Set the device format based upon settings.  This will be used for format conversion.
	// This also sets the frame I/O function.
	if (port->streamCount > 1) {
#if DEBUG > 0
		printf("OSX HW is non-interleaved %d streams (%u channel)\n",
			   port->streamCount, port->deviceFormat.mChannelsPerFrame);
#endif
		deviceFormat |= MUS_NON_INTERLEAVED;
		_impl->port[REC].frameFn = Impl::Port::noninterleavedGetFrames;
		_impl->port[PLAY].frameFn = Impl::Port::noninterleavedSendFrames;
	}
	else {
#if DEBUG > 0
		assert(port->streamCount == 1);
		printf("OSX HW is 1 interleaved stream (%u channel)\n",
			   port->deviceFormat.mChannelsPerFrame);
#endif
		deviceFormat |= MUS_INTERLEAVED;
		_impl->port[REC].frameFn = Impl::Port::interleavedGetFrames;
		_impl->port[PLAY].frameFn = Impl::Port::interleavedSendFrames;
	}
#if DEBUG > 0
	printf("OSX setting non-direction-specific device params to %d channels\n", _impl->port[portIndex].virtualChannels);
#endif

	setDeviceParams(deviceFormat,
					_impl->port[portIndex].virtualChannels,
					port->deviceFormat.mSampleRate);
	return 0;
}

int OSXAudioDevice::doSetQueueSize(int *pWriteSize, int *pCount)
{
	ENTER(OSXAudioDevice::doSetQueueSize());
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
		*pWriteSize = port->audioBufFrames / *pCount;

		port->deviceBufFrames = deviceBufferBytes / (port->deviceFormat.mChannelsPerFrame * sizeof(float));

#if DEBUG > 0
		printf("%s device bufsize: %d bytes (%d frames). circ buffer %d frames\n",
				dir == REC ? "Input" : "Output", deviceBufferBytes,
				port->deviceBufFrames, port->audioBufFrames);
		printf("\tBuffer configured for %d channels of audio\n",
			   port->audioBufChannels * port->streamCount);
#endif
		int buflen = port->audioBufFrames * port->audioBufChannels * port->streamCount;
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

int OSXAudioDevice::getRecordDeviceChannels() const
{
	return _impl->port[REC].virtualChannels;
}

int OSXAudioDevice::getPlaybackDeviceChannels() const
{
	return _impl->port[PLAY].virtualChannels;
}

int	OSXAudioDevice::doGetFrames(void *frameBuffer, int frameCount)
{
	const int frameChans = getFrameChannels();
	Impl::Port *port = &_impl->port[REC];
	return (*port->frameFn)(port, frameBuffer, frameCount, frameChans);
}

int	OSXAudioDevice::doSendFrames(void *frameBuffer, int frameCount)
{
	const int frameChans = getFrameChannels();
	Impl::Port *port = &_impl->port[PLAY];
	return (*port->frameFn)(port, frameBuffer, frameCount, frameChans);
}

int OSXAudioDevice::doGetFrameCount() const
{
	return _impl->frameCount;
}

bool OSXAudioDevice::recognize(const char *desc)
{
//	return (desc == NULL) || (strncmp(desc, "OSXHW", 5) == 0);
	return true;
}

AudioDevice *OSXAudioDevice::create(const char *inputDesc, const char *outputDesc, int mode)
{
	return new OSXAudioDevice(inputDesc ? inputDesc : outputDesc);
}

// Static utilities

static const char *
errToString(OSStatus err)
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

static OSStatus
getDeviceList(AudioDeviceID **devList, int *devCount)
{
   UInt32 size;

   OSStatus err = AudioHardwareGetPropertyInfo(
                        kAudioHardwarePropertyDevices,
                        &size, NULL);
   if (err != kAudioHardwareNoError) {
      printf("Can't get hardware device list property info.\n");
      return err;
   }
   *devCount = size / sizeof(AudioDeviceID);
   *devList = new AudioDeviceID[*devCount];
   err = AudioHardwareGetProperty(
                        kAudioHardwarePropertyDevices,
                        &size, *devList);
   if (err != kAudioHardwareNoError) {
      printf("Can't get hardware device list.\n");
      return err;
   }

   return 0;
}

static AudioDeviceID
findDeviceID(const char *devName, AudioDeviceID *devList, int devCount, Boolean isInput)
{
	AudioDeviceID devID = 0;
    UInt32 size = sizeof(devID);
	if (!strcasecmp(devName, "default")) {
		OSStatus err = AudioHardwareGetProperty(
							isInput ?
								kAudioHardwarePropertyDefaultInputDevice :
									kAudioHardwarePropertyDefaultOutputDevice,
							&size,
				   			(void *) &devID);
		if (err != kAudioHardwareNoError || devID == kAudioDeviceUnknown) {
			fprintf(stderr, "Cannot find default OSX device: %s\n", ::errToString(err));
			return 0;
		}
		return devID;
	}
	for (int dev = 0; dev < devCount && devID == 0; ++dev) {

	   OSStatus err = AudioDeviceGetPropertyInfo(devList[dev],
                                            	 0,
                                            	 isInput,
                                            	 kAudioDevicePropertyDeviceName,
                                            	 &size, NULL);
	   if (err != kAudioHardwareNoError) {
    	  fprintf(stderr, "findDeviceID: Can't get device name property info for device %lu.\n",
                  devList[dev]);
    	  continue;
	   }

	   char *name = new char[size + 1];
	   err = AudioDeviceGetProperty(devList[dev],
                                	0,
                                	isInput,
                                	kAudioDevicePropertyDeviceName,
                                	&size, name);
	   if (err != kAudioHardwareNoError) {
    	  fprintf(stderr, "findDeviceID: Can't get device name property for device %lu.\n",
			  	  devList[dev]);
		  delete [] name;
    	  continue;
	   }
#if DEBUG > 0
		printf("Checking device %d -- name: \"%s\"\n", dev, name);
#endif
	   // For now, we must match the case as well because strcasestr() does not exist.
	   if (strstr(name, devName) != NULL) {
#if DEBUG > 0
			printf("MATCH FOUND\n");
#endif
			devID = devList[dev];
	   }
	   delete [] name;
	}
	return devID;
}

#endif	// MACOSX
