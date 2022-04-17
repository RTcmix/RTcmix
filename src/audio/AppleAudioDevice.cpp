//
//  AppleAudioDevice.cpp
//  Created by Douglas Scott on 11/12/13.
//
//

#include "AppleAudioDevice.h"
#include <AudioUnit/AudioUnit.h>
#include <MacTypes.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ugens.h>
#include <sndlibsupport.h>	// RTcmix header
#include <RTSemaphore.h>
#include <new>
#include <syslog.h>

#ifdef STANDALONE

#include <CoreAudio/CoreAudio.h>
#include <CoreAudio/AudioHardware.h>
#include <CoreFoundation/CFRunLoop.h>

// Test code to see if it is useful to make sure HW and render threads do not
// access the circular buffer at the same time.  Does not seem to be necessary.
#define LOCK_IO 1

// OSX runs the rendering code in its own thread
#define RENDER_IN_CALLBACK 0
// This is true for OSX.
#define OUTPUT_CALLBACK_FIRST 1

#else	/* IOS */

#define LOCK_IO 0	/* never for iOS */
#define RENDER_IN_CALLBACK 1
#define OUTPUT_CALLBACK_FIRST 0

#endif

#if LOCK_IO
#include <Lockable.h>
#endif


#define DEBUG 0

#if DEBUG > 0
	#if !defined(IOSDEV)
		#define DERROR(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
		#define DPRINT(fmt, ...) printf(fmt, ## __VA_ARGS__)
		#define ENTER(fun) printf("Entering " #fun "\n");
			#if DEBUG > 1
				#define DPRINT1(fmt, ...) printf(fmt, ## __VA_ARGS__)
			#else
				#define DPRINT1(fmt, ...)
			#endif
	#else	/* IOS */
		#define DERROR(fmt, ...) syslog(LOG_ERR, fmt, ## __VA_ARGS__)
		#define DPRINT(fmt, ...) syslog(LOG_NOTICE, fmt, ## __VA_ARGS__)
		#define ENTER(fun) syslog(LOG_NOTICE, "Entering " #fun "\n");
			#if DEBUG > 1
				#define DPRINT1(fmt, ...) syslog(LOG_NOTICE, fmt, ## __VA_ARGS__)
			#else
				#define DPRINT1(fmt, ...)
			#endif
	#endif	/* IOS */
#else	/* DEBUG !> 0 */
	#define ENTER(fun)
	#define DPRINT(fmt, ...)
	#define DPRINT1(fmt, ...)
	#if !defined(IOSDEV)
	#define DERROR(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
	#else
	#define DERROR(fmt, ...) syslog(LOG_ERR, fmt, ## __VA_ARGS__)
	#endif
#endif

// Utilities

inline int max(int x, int y) { return (x >= y) ? x : y; }
inline int min(int x, int y) { return (x < y) ? x : y; }
inline int inAvailable(int filled, int size) {
	return size - filled;
}

const int kOutputBus = 0;
const int kInputBus = 1;
static const int REC = 0, PLAY = 1;

static const char *errToString(OSStatus err);
#ifdef STANDALONE
static AudioDeviceID findDeviceID(const char *devName, AudioDeviceID *devList,
								  int devCount, Boolean isInput);
#endif

struct AppleAudioDevice::Impl {
    Impl();
    ~Impl();

#ifdef STANDALONE
	AudioDeviceID			*deviceIDs;				// Queried on system.
	int						deviceCount;			// Queried on system.
	char 					*deviceName;			// Passed in by user.
	AudioDeviceID			deviceID;
	Float64					savedDeviceSampleRate;	// For reset
#endif
	struct Port {
		int						streamIndex;		// Which stream
		int						streamCount;		// How many streams open
		int						streamChannel;		// 1st chan of first stream
		SInt32					*channelMap;		// Channel map
		int						channelMapCount;	// number of channels in the map
		AudioBufferList			*streamDesc;
		unsigned int 			deviceBufFrames;	// hw buf length
		float					*audioBuffer;		// circ. buffer
		int						audioBufFrames;		// length of audioBuffers
		int						audioBufChannels;	// channels in audioBuffers
		int						virtualChannels;	// what is reported publically (may vary based on mono-stereo)
		int						inLoc, outLoc;		// circ. buffer indices
		int						audioBufFilled;		// audioBuffer samples available
#if LOCK_IO
        Lockable                lock;
#endif
		int						getFrames(void *, int, int);
		int						sendFrames(void *, int, int);
	} 						port[2];
	AudioStreamBasicDescription deviceFormat;		// format
	AudioStreamBasicDescription clientFormat;		// format
	int						bufferSampleFormat;
	int						frameCount;
	bool					gotFormatNotification;
	bool					paused;
	bool					stopping;
	bool					recording;				// Used by OSX code
	bool					playing;				// Used by OSX code
	int						inputFramesAvailable;
	pthread_t               renderThread;
    RTSemaphore *           renderSema;
    int                     underflowCount;
	AudioUnit				audioUnit;
	AudioBufferList			*inputBufferList;

	AudioBufferList *		createInputBufferList(int inChannelCount, bool inInterleaved);
	void					setBufferList(AudioBufferList *inList);
	void					destroyInputBufferList(AudioBufferList *inList);
    int                     startRenderThread(AppleAudioDevice *parent);
    void                    stopRenderThread();
	inline int				outputDeviceChannels() const;
	static OSStatus			audioUnitInputCallback(void *inUserData,
													AudioUnitRenderActionFlags *ioActionFlags,
													const AudioTimeStamp *inTimeStamp,
													UInt32 inBusNumber,
													UInt32 inNumberFrames,
													AudioBufferList *ioData);
	static OSStatus			audioUnitRenderCallback(void *inUserData,
											AudioUnitRenderActionFlags *ioActionFlags,
											const AudioTimeStamp *inTimeStamp,
											UInt32 inBusNumber,
											UInt32 inNumberFrames,
											AudioBufferList *ioData);
    static void *			renderProcess(void *context);
	static void				propertyListenerProc(void *inRefCon,
												 AudioUnit inUnit,
												 AudioUnitPropertyID inID,
												 AudioUnitScope inScope,
												 AudioUnitElement inElement);
};

// I/O Functions

int
AppleAudioDevice::Impl::Port::getFrames(void *frameBuffer, int inFrameCount, int inFrameChans)
{
	const int bufFrames = audioBufFrames;
	int bufLoc = 0;
	float **fFrameBuffer = (float **) frameBuffer;		// non-interleaved
	DPRINT1("AppleAudioDevice::doGetFrames: inFrameCount = %d REC filled = %d\n", inFrameCount, audioBufFilled);
#if LOCK_IO
    lock.lock();
#endif
	int streamsToCopy = 0;
#ifdef STANDALONE
	assert(inFrameCount <= audioBufFilled);
#else
	if (inFrameCount <= audioBufFilled)		// For IOS, we need to handle the case where there is no audio to pull.
#endif
		streamsToCopy = inFrameChans < streamCount ? inFrameChans : streamCount;
	
	DPRINT("AppleAudioDevice::doGetFrames: Copying %d non-interleaved internal bufs into %d-channel user frame\n",
		   streamsToCopy, inFrameChans);
	int stream;
	for (stream = 0; stream < streamsToCopy; ++stream) {
		// Offset into serially-ordered, multi-channel non-interleaved buf.
		register float *buf = &audioBuffer[stream * bufFrames];
		float *frame = fFrameBuffer[stream];
		bufLoc = outLoc;
		DPRINT1("\tstream %d: raw offset into mono internal buffer: %ld (%d * %d)\n", stream, buf - &audioBuffer[0], stream, bufFrames);
		DPRINT1("\tread internal (already-offset) buf starting at outLoc %d\n", bufLoc);
		// Write each monaural frame from circ. buffer into a non-interleaved output frame.
		for (int out=0; out < inFrameCount; ++out) {
			if (bufLoc >= bufFrames)
				bufLoc -= bufFrames;	// wrap
			frame[out] = buf[bufLoc++];
		}
	}
	// Zero out any remaining frame channels
	for (; stream < inFrameChans; ++stream) {
		DPRINT("Zeroing user frame channel %d\n", stream);
		memset(fFrameBuffer[stream], 0, inFrameCount * sizeof(float));
	}
	outLoc = bufLoc;
	audioBufFilled -= inFrameCount;
#if LOCK_IO
    lock.unlock();
#endif
	DPRINT1("\tREC Filled now %d\n", audioBufFilled);
	DPRINT1("\tREC bufLoc ended at %d. Returning frameCount = %d\n", bufLoc, inFrameCount);
	return inFrameCount;
}

int
AppleAudioDevice::Impl::Port::sendFrames(void *frameBuffer, int frameCount, int frameChans)
{
	float **fFrameBuffer = (float **) frameBuffer;		// non-interleaved
	const int bufFrames = audioBufFrames;
	int loc = 0;
	DPRINT("AppleAudioDevice::Impl::Port::sendFrames: frameCount = %d, PLAY filled = %d\n", frameCount, audioBufFilled);
	DPRINT("\tCopying %d channel rendered frame into %d non-interleaved internal buf channels\n", frameChans, streamCount);
#if LOCK_IO
    lock.lock();
#endif
	for (int stream = 0; stream < streamCount; ++stream) {
		loc = inLoc;
		float *frame = fFrameBuffer[stream];
		// Offset into serially-ordered, multi-channel non-interleaved buf.
		float *buf = &audioBuffer[stream * bufFrames];
		if (stream < frameChans) {
			// Write each non-interleaved input frame into circular buf.
			for (int in=0; in < frameCount; ++in) {
				if (loc >= bufFrames)
					loc -= bufFrames;	// wrap
				buf[loc++] = frame[in];
			}
		}
		else {
			DPRINT("Zeroing internal buf channel %d\n", stream);
			for (int in=0; in < frameCount; ++in) {
				if (loc >= bufFrames)
					loc -= bufFrames;	// wrap
				buf[loc++] = 0.0f;
			}
		}
	}
	audioBufFilled += frameCount;
	inLoc = loc;
#if LOCK_IO
    lock.unlock();
#endif
	DPRINT1("\tPLAY Filled up to %d\n", audioBufFilled);
    DPRINT1("\tLeaving AppleAudioDevice::Impl::Port::sendFrames:  PLAY inLoc ended at %d. Returning frameCount = %d\n", inLoc, frameCount);
	return frameCount;
}

AppleAudioDevice::Impl::Impl()
:
#if defined(STANDALONE)
deviceIDs(NULL), deviceName(NULL), deviceID(0), savedDeviceSampleRate(0.0),
#endif
renderThread(NULL), renderSema(NULL), underflowCount(0)
{
    frameCount = 0;
	gotFormatNotification = false;
	paused = false;
	stopping = false;
	recording = false;
	playing = false;
	inputFramesAvailable = 0;
	inputBufferList = NULL;
	for (int n = REC; n <= PLAY; ++n) {
		port[n].streamIndex = 0;
		port[n].streamCount = 0;		// Indicates that we are using the default.
		port[n].streamChannel = 0;
		port[n].channelMap = NULL;
		port[n].channelMapCount = 0;
		port[n].streamDesc = NULL;
		port[n].deviceBufFrames = 0;
		port[n].audioBufFrames = 0;
		port[n].audioBufChannels = 0;
		port[n].virtualChannels = 0;
		port[n].audioBuffer = NULL;
		port[n].inLoc = port[n].outLoc = 0;
		port[n].audioBufFilled = 0;
	}
}

AppleAudioDevice::Impl::~Impl()
{
	delete [] port[REC].channelMap;
	delete [] port[PLAY].channelMap;
    delete [] (char *) port[REC].streamDesc;
	delete [] port[REC].audioBuffer;
	delete [] (char *) port[PLAY].streamDesc;
	delete [] port[PLAY].audioBuffer;
#if defined(STANDALONE)
	delete [] deviceIDs;
	delete [] deviceName;
#endif
    delete renderSema;
	destroyInputBufferList(inputBufferList);
}

AudioBufferList *	AppleAudioDevice::Impl::createInputBufferList(int inChannelCount, bool inInterleaved)
{
	Impl::Port *port = &this->port[REC];
	int numBuffers = (inInterleaved) ? 1 : inChannelCount;
	int byteSize = sizeof(AudioBufferList) + (numBuffers - 1) * sizeof(AudioBufferList);
	void *rawmem = new char[byteSize];
	DPRINT("AppleAudioDevice::Impl::createInputBufferList creating AudioBufferList with %d %d-channel buffers\n",
		   numBuffers, port->audioBufChannels);
	AudioBufferList *newList = new (rawmem) AudioBufferList;
	newList->mNumberBuffers = numBuffers;
	// Set up constant values for each buffer from our port's state
	for (int n = 0; n < numBuffers; ++n) {
		newList->mBuffers[n].mNumberChannels = port->audioBufChannels;
	}
	return newList;
}

void	AppleAudioDevice::Impl::setBufferList(AudioBufferList *inList)
{
	Impl::Port *port = &this->port[REC];
	// Point each buffer's data pointer at the appropriate offset into our port audioBuffer and set the byte size.
	for (int n = 0; n < inList->mNumberBuffers; ++n) {
		DPRINT1("\tbuffer[%d] mData set to stream frame offset %d, local offset %d, length %d\n",
				n, n * port->audioBufFrames, port->inLoc, port->deviceBufFrames);
		inList->mBuffers[n].mData = (void *) &port->audioBuffer[(n * port->audioBufFrames) + port->inLoc];
		inList->mBuffers[n].mDataByteSize = port->deviceBufFrames * sizeof(float);
	}
}

void	AppleAudioDevice::Impl::destroyInputBufferList(AudioBufferList *inList)
{
	// delete in reverse of how we created it
	inList->~AudioBufferList();
	char *rawmem = (char *) inList;
	delete [] rawmem;
}

void *
AppleAudioDevice::Impl::renderProcess(void *context)
{
	AppleAudioDevice *device = (AppleAudioDevice *) context;
	AppleAudioDevice::Impl *impl = device->_impl;
	if (setpriority(PRIO_PROCESS, 0, -20) != 0)
	{
        //	perror("AppleAudioDevice::Impl::renderProcess: Failed to set priority of thread.");
	}
    while (true) {
        DPRINT("AppleAudioDevice::Impl::renderProcess waiting...\n");
        impl->renderSema->wait();
        if (impl->stopping) {
            DPRINT("\nAppleAudioDevice::Impl::renderProcess woke to stop -- breaking out\n");
            break;
        }
#if DEBUG > 0
        if (impl->underflowCount > 0) {
            DPRINT("\nAppleAudioDevice::Impl::renderProcess woke up -- underflow count %d -- running slice\n", impl->underflowCount);
            --impl->underflowCount;
        }
		else {
			DPRINT("\nAppleAudioDevice::Impl::renderProcess woke up -- running slice\n");
		}
#endif
        bool ret = device->runCallback();
        if (ret == false) {
			DPRINT("AppleAudioDevice: renderProcess: run callback returned false -- breaking out\n");
            break;
        }
    }
    DPRINT("AppleAudioDevice: renderProcess: calling stop callback\n");
    device->stopCallback();
    DPRINT("AppleAudioDevice: renderProcess exiting\n");
	return NULL;
}

OSStatus AppleAudioDevice::Impl::audioUnitInputCallback(void *inUserData,
											   AudioUnitRenderActionFlags *ioActionFlags,
											   const AudioTimeStamp *inTimeStamp,
											   UInt32 inBusNumber,
											   UInt32 inNumberFrames,
											   AudioBufferList *ioData)
{
	AppleAudioDevice *device = (AppleAudioDevice *) inUserData;
	Impl *impl = device->_impl;
	Port *port = &impl->port[REC];
	OSStatus result = noErr;
	DPRINT("\nAppleAudioDevice: top of audioUnitInputCallback: inBusNumber: %d inNumberFrames: %u\n", (int)inBusNumber, (unsigned)inNumberFrames);
	assert (inBusNumber == kInputBus);
	assert (impl->recording);

#if LOCK_IO
	if (!port->lock.tryLock()) {
		DPRINT("AppleAudioDevice: record section skipped due to block on render thread\n");
		return noErr;
	}
#endif

	DPRINT("\tCopying directly into port's audioBuffer via AudioBuffer\n");
	// We supply our own AudioBufferList for input.  Its buffers point into our port's buffer
	impl->setBufferList(impl->inputBufferList);
	// Pull audio from hardware into our AudioBufferList
	result = AudioUnitRender(impl->audioUnit, ioActionFlags, inTimeStamp, kInputBus, inNumberFrames, impl->inputBufferList);
	if (result != noErr) {
		DERROR("ERROR: audioUnitInputCallback: AudioUnitRender failed with status %d\n", (int)result);
		return result;
	}
	
	// If we are only recording, we are done here except for notifying the RTcmix rendering thread, or running the render.
	
	
	port->audioBufFilled += inNumberFrames;
	port->inLoc = (port->inLoc + inNumberFrames) % (port->audioBufFrames * port->audioBufChannels);
	
	DPRINT1("\tREC Filled = %d\n", port->audioBufFilled);
	DPRINT1("\tREC inLoc ended at %d\n", port->inLoc);

#if LOCK_IO
	port->lock.unlock();
#endif

	// If we are only recording and not playing, we wake the render thread (or run the render call) here.
	// Otherwise, if we are playing as well, and the output callback did not precede us, we wait to do the render there.
	
	if (!impl->playing) {
		
		impl->frameCount += inNumberFrames;
#if RENDER_IN_CALLBACK
		DPRINT("\tRunning render callback inline\n");
        bool ret = device->runCallback();
        if (ret == false) {
			DPRINT("\tRun callback returned false -- calling stop callback\n");
			device->stopCallback();
        }
#else

#if !OUTPUT_CALLBACK_FIRST
		DPRINT("\tWaking render thread\n");
		impl->renderSema->post();
#endif
#endif
	}
#if OUTPUT_CALLBACK_FIRST
	DPRINT("\tWaking render thread\n");
	impl->renderSema->post();
#endif
	
	return result;
}

OSStatus AppleAudioDevice::Impl::audioUnitRenderCallback(void *inUserData,
												AudioUnitRenderActionFlags *ioActionFlags,
												const AudioTimeStamp *inTimeStamp,
												UInt32 inBusNumber,
												UInt32 inNumberFrames,
												AudioBufferList *ioData)
{
	AppleAudioDevice *device = (AppleAudioDevice *) inUserData;
	Impl *impl = device->_impl;
	int framesAdvanced = 0;
	Port *port;
	DPRINT("\nAppleAudioDevice: top of audioUnitRenderCallback: inBusNumber: %d inNumberFrames: %u\n", (int)inBusNumber, (unsigned)inNumberFrames);
	assert (inBusNumber == kOutputBus);
	if (impl->playing) {
		// Copy audio from our rendered buffer(s) into ioData to be written to the hardware
		port = &impl->port[PLAY];
#if LOCK_IO
        if (!port->lock.tryLock()) {
            DPRINT("AppleAudioDevice: playback section skipped due to block on render thread\n");
            goto End;
        }
#endif
//		const int framesToWrite = port->deviceBufFrames;
		// NOTE TO ME:  Cannot guarantee that we will have space for deviceBufFrames worth of audio in output
		const int framesToWrite = inNumberFrames;
		const int destchans = 1;	// non-interleaved output - change this if we use interleaved
		const int srcchans = port->audioBufChannels;
		const int bufLen = port->audioBufFrames * srcchans;
		int framesAvail = port->audioBufFilled;
		
		DPRINT("AppleAudioDevice: playback section (out buffer %d)\n", port->streamIndex);
		DPRINT1("framesAvail (Filled) = %d\n", framesAvail);
		if (framesAvail < framesToWrite) {
#if DEBUG > 0
            if ((impl->underflowCount %4) == 0) {
                DERROR("AppleAudioDevice (playback): framesAvail (%d) < needed (%d) -- UNDERFLOW\n", framesAvail, framesToWrite);
                DPRINT("\tzeroing input buffer and going on\n");
            }
            ++impl->underflowCount;
#endif
            for (int stream = 0; stream < port->streamCount; ++stream) {
                memset(&port->audioBuffer[stream * port->audioBufFrames], 0, bufLen * sizeof(float));
            }
            framesAvail = framesToWrite;
            port->audioBufFilled += framesToWrite;
		}
		
#if RENDER_IN_CALLBACK
		// If we are running inTraverse synchronously, we run it before we copy the data to the port's audio buffers
#if OUTPUT_CALLBACK_FIRST
		if (!impl->recording)
#endif
		{
			DPRINT("\tRunning render callback inline\n");
			bool ret = device->runCallback();
			if (ret == false) {
				DPRINT("\tRun callback returned false -- calling stop callback\n");
				device->stopCallback();
			}
		}
#endif	/* RENDER_IN_CALLBACK */
        DPRINT1("\tPLAY outLoc begins at %d (out of %d)\n",
               port->outLoc, bufLen);
        int framesDone = 0;
        // Audio data has been written into port->audioBuffer during doSendFrames.
        //   Treat it as circular buffer.
        // Copy that audio into the ioData buffer pointers.  Channel counts always match.
        while (framesDone < framesToWrite) {
            int bufLoc = port->outLoc;
            DPRINT("\tLooping for %d (%d-channel) stream%s\n",
                   port->streamCount, destchans, port->streamCount > 1 ? "s" : "");
            for (int stream = 0; stream < port->streamCount; ++stream) {
                const int strIdx = stream + port->streamIndex;
                register float *src = &port->audioBuffer[stream * port->audioBufFrames];
                register float *dest = (float *) ioData->mBuffers[strIdx].mData;
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
#if LOCK_IO
			port->lock.unlock();
#endif
       }
        DPRINT1("\tPLAY Filled down to %d\n", port->audioBufFilled);
        DPRINT1("\tPLAY outLoc ended at %d\n", port->outLoc);
	}
	impl->frameCount += framesAdvanced;
#if OUTPUT_CALLBACK_FIRST
	if (!impl->recording)
#endif
	{
#if !RENDER_IN_CALLBACK
		DPRINT("\tWaking render thread\n");
    	impl->renderSema->post();
#endif
	}
End:
	DPRINT("AppleAudioDevice: leaving audioUnitRenderCallback\n\n");
	return noErr;
}

union Prop { Float64 f64; Float32 f32; UInt32 uint; SInt32 sint; AudioStreamBasicDescription desc; };
#if DEBUG > 0
static const char *scopeToName(AudioUnitScope scope)
{
	switch (scope) {
	case kAudioUnitScope_Input:
    	return "Input";
	case kAudioUnitScope_Output:
    	return "Output";
	default:
		return "Other";
	}
}
static const char *elementToName(AudioUnitElement element)
{
	switch (element) {
		case 0:
			return "Output";
		case 1:
			return "Input";
		default:
			return "Other";
	}
}
#endif

void AppleAudioDevice::Impl::propertyListenerProc(void *inRefCon,
											 AudioUnit inUnit,
											 AudioUnitPropertyID inID,
											 AudioUnitScope inScope,
											 AudioUnitElement inElement)
{
	DPRINT1(">>>> propertyListenerProc called for prop %d, scope: %s, bus: %s\n",
		   (int)inID, scopeToName(inScope), elementToName(inElement));
	AppleAudioDevice *device = (AppleAudioDevice *) inRefCon;
	Impl *impl = device->_impl;
	Prop theProp;
	UInt32 size = sizeof(theProp);
	OSStatus status = AudioUnitGetProperty(impl->audioUnit,
								  inID,
								  inScope,
								  inElement,
								  &theProp,
								  &size);
	if (status != noErr) {
		DERROR(">>>> AppleAudioDevice: failed to retrieve property during listener proc: error %d\n", (int)status);
		return;
	}
	bool needToStop = false;
	switch (inID) {
		case kAudioUnitProperty_SampleRate:
			DPRINT(">>>> AppleAudioDevice: got sample rate notification: %f\n", theProp.f64);
			if (device->isRunning() && theProp.f64 != device->getSamplingRate()) {
				needToStop = true;
			}
			impl->gotFormatNotification = true;
			break;
		case kAudioUnitProperty_StreamFormat:
#ifdef STANDALONE
		case kAudioDevicePropertyStreamFormat:
#endif
			if (!impl->gotFormatNotification &&
					((inScope == kAudioUnitScope_Output && inElement == 0) || (inScope == kAudioUnitScope_Input && inElement == 1))) {
				DPRINT(">>>> AppleAudioDevice: got hardware stream format notification -- sr = %f\n", theProp.desc.mSampleRate);
				impl->gotFormatNotification = true;
			}
			if (device->isRunning() && theProp.desc.mSampleRate != device->getSamplingRate()) {
				needToStop = true;
			}
			break;
#ifdef STANDALONE
		case kAudioDevicePropertyBufferFrameSize:
			DPRINT(">>>> AppleAudioDevice: got buffer frame size notification: %u\n", (unsigned)theProp.uint);
			if (device->isRunning() && theProp.uint != impl->port[!impl->recording].deviceBufFrames) {
				needToStop = true;
			}
			break;
#endif
		case kAudioUnitProperty_LastRenderError:
			DPRINT(">>>> AppleAudioDevice: got render error notification: %d\n", (int)theProp.sint);
			break;
	}
	if (needToStop) {
		fprintf(stderr, "Device format changed - stopping\n");
		syslog(LOG_ALERT, "Device format changed - stopping\n");
		device->error("Device format changed - stopping");
		device->stop();
	}
}

int AppleAudioDevice::Impl::startRenderThread(AppleAudioDevice *parent)
{
	DPRINT("\tAppleAudioDevice::Impl::startRenderThread: starting thread\n");
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int status = pthread_attr_setschedpolicy(&attr, SCHED_RR);
	if (status != 0) {
		DERROR("startRenderThread: Failed to set scheduling policy\n");
	}
	status = pthread_create(&renderThread, &attr, renderProcess, parent);
	pthread_attr_destroy(&attr);
	if (status < 0) {
		DERROR("Failed to create render thread");
	}
	return status;
}

void AppleAudioDevice::Impl::stopRenderThread()
{
    assert(renderThread != 0);	// should not get called again!
	stopping = true;
    DPRINT("AppleAudioDevice::Impl::stopRenderThread: posting to semaphore for thread\n");
    renderSema->post();  // wake up, it's time to die
    DPRINT("AppleAudioDevice::Impl::stopRenderThread: waiting for thread to finish\n");
    if (pthread_join(renderThread, NULL) == -1) {
        DERROR("AppleAudioDevice::Impl::stopRenderThread: terminating thread!\n");
        pthread_cancel(renderThread);
        renderThread = 0;
    }
    DPRINT("\tAppleAudioDevice::Impl::stopRenderThread: thread done\n");
}

// Main class implementation

AppleAudioDevice::AppleAudioDevice(const char *desc) : _impl(new Impl)
{
	parseDeviceDescription(desc);
}

AppleAudioDevice::~AppleAudioDevice()
{
	//DPRINT("AppleAudioDevice::~AppleAudioDevice()\n");
	close();
	AudioUnitUninitialize(_impl->audioUnit);
	AudioComponentInstanceDispose(_impl->audioUnit);
	delete _impl;
}

int AppleAudioDevice::doOpen(int mode)
{
	ENTER(AppleAudioDevice::doOpen());
	AppleAudioDevice::Impl *impl = _impl;
	OSStatus status;

	if (mode & Passive) {
		return error("AppleAudioDevice do not support passive device mode");
	}
	impl->gotFormatNotification = false;
	impl->recording = ((mode & Record) != 0);
	impl->playing = ((mode & Playback) != 0);

	// Describe audio component
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
#ifdef IOS
	desc.componentSubType = kAudioUnitSubType_RemoteIO;		// iOS
#else
	desc.componentSubType = kAudioUnitSubType_HALOutput;	// OSX
#endif
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	
	AudioComponent comp = AudioComponentFindNext(NULL, &desc);
	status = AudioComponentInstanceNew(comp, &impl->audioUnit);
	if (status != noErr) {
		return appleError("Unable to create the audio unit", status);
	}

#ifdef STANDALONE
    // Set the device for the HAL Audio Unit
    
    Boolean isInput = impl->recording && !impl->playing;
    AudioDeviceID devID = ::findDeviceID(impl->deviceName, impl->deviceIDs,
                                         impl->deviceCount, isInput);
    
    if (devID == 0) {
        char msg[64];
        snprintf(msg, 64, "No matching device found for '%s'\n", impl->deviceName);
        return error(msg);
    }
    
    status = AudioUnitSetProperty(impl->audioUnit,
                                  kAudioOutputUnitProperty_CurrentDevice,
                                  kAudioUnitScope_Global,
                                  0,
                                  &devID,
                                  sizeof(devID));
    if (status != noErr) {
        return appleError("Unable to set hardware device on audio unit", status);
    }
    
    // Set up property listeners
    
    status = AudioUnitAddPropertyListener(impl->audioUnit, kAudioDevicePropertyBufferFrameSize, Impl::propertyListenerProc, this);
    if (status != noErr) {
        return appleError("Unable to set BufferFrameSize listener on audio unit", status);
    }
    status = AudioUnitAddPropertyListener(impl->audioUnit, kAudioDevicePropertyStreamFormat, Impl::propertyListenerProc, this);
    if (status != noErr) {
        return appleError("Unable to set Device StreamFormat listener on audio unit", status);
    }
    
#endif

    UInt32 enableInput = impl->recording;
    UInt32 enableOutput = impl->playing;
    if (enableInput) {
        UInt32 supportsInput = 0;
        UInt32 uSize = sizeof(supportsInput);
        status = AudioUnitGetProperty(impl->audioUnit,
                                      kAudioOutputUnitProperty_HasIO,
                                      kAudioUnitScope_Input,
                                      kInputBus,
                                      &supportsInput,
                                      &uSize);
        if (status != noErr || !supportsInput) {
            return error("Selected audio device does not support input - use aggregate device");
        }
    }
    if (enableOutput) {
        UInt32 supportsOutput = 0;
        UInt32 uSize = sizeof(supportsOutput);
        status = AudioUnitGetProperty(impl->audioUnit,
                                      kAudioOutputUnitProperty_HasIO,
                                      kAudioUnitScope_Output,
                                      kOutputBus,
                                      &supportsOutput,
                                      &uSize);
        if (status != noErr || !supportsOutput) {
            return error("Selected audio device does not support output - use aggregate device");
        }
    }
	// Enable IO for playback and/or record.  This is done before setting the (OSX) device.
    	
	status = AudioUnitSetProperty(impl->audioUnit,
								  kAudioOutputUnitProperty_EnableIO,
								  kAudioUnitScope_Input,
								  kInputBus,
								  &enableInput,
								  sizeof(enableInput));

	if (status != noErr) {
		return appleError("Unable to Enable/Disable input I/O on audio unit", status);
	}
	status = AudioUnitSetProperty(impl->audioUnit,
								  kAudioOutputUnitProperty_EnableIO,
								  kAudioUnitScope_Output,
								  kOutputBus,
								  &enableOutput,
								  sizeof(enableOutput));
	if (status != noErr) {
		return appleError("Unable to Enable/Disable output I/O on audio unit", status);
	}

	status = AudioUnitAddPropertyListener(impl->audioUnit, kAudioUnitProperty_SampleRate, Impl::propertyListenerProc, this);
	if (status != noErr) {
		return appleError("Unable to set SampleRate listener on audio unit", status);
	}
	status = AudioUnitAddPropertyListener(impl->audioUnit, kAudioUnitProperty_StreamFormat, Impl::propertyListenerProc, this);
	if (status != noErr) {
		return appleError("Unable to set AU StreamFormat listener on audio unit", status);
	}
	status = AudioUnitAddPropertyListener(impl->audioUnit, kAudioUnitProperty_LastRenderError, Impl::propertyListenerProc, this);
	if (status != noErr) {
		return appleError("Unable to set LastRenderError listener on audio unit", status);
	}
	// Get HW format and set I/O callback for each direction.
	if (impl->recording) {
		UInt32 size = sizeof(AudioStreamBasicDescription);
		status = AudioUnitGetProperty(_impl->audioUnit,
									  kAudioUnitProperty_StreamFormat,
									  kAudioUnitScope_Input,			// HW side
									  kInputBus,
									  &impl->deviceFormat,
									  &size);
		if (status != noErr)
			return appleError("Cannot read input HW stream format", status);
		DPRINT("input HW format: sr %f fmt %d flags 0x%x fpp %d cpf %d bpc %d bpp %d bpf %d\n",
			   impl->deviceFormat.mSampleRate, (int)impl->deviceFormat.mFormatID, (unsigned)impl->deviceFormat.mFormatFlags,
			   (int)impl->deviceFormat.mFramesPerPacket, (int)impl->deviceFormat.mChannelsPerFrame, (int)impl->deviceFormat.mBitsPerChannel,
			   (int)impl->deviceFormat.mBytesPerPacket, (int)impl->deviceFormat.mBytesPerFrame);
		AURenderCallbackStruct proc = { Impl::audioUnitInputCallback, this };
		status = AudioUnitSetProperty(impl->audioUnit,
									  kAudioOutputUnitProperty_SetInputCallback,
									  kAudioUnitScope_Output,
									  kInputBus,
									  &proc,
									  sizeof(AURenderCallbackStruct));
		if (status != noErr) {
			return appleError("Unable to set input callback", status);
		}
	}
	if (impl->playing) {
		UInt32 size = sizeof(AudioStreamBasicDescription);
		status = AudioUnitGetProperty(_impl->audioUnit,
									  kAudioUnitProperty_StreamFormat,
									  kAudioUnitScope_Output,			// HW side
									  kOutputBus,
									  &impl->deviceFormat,
									  &size);
		if (status != noErr)
			return appleError("Cannot read output HW stream format", status);
		DPRINT("output HW format: sr %f fmt %d flags 0x%x fpp %d cpf %d bpc %d bpp %d bpf %d\n",
			   impl->deviceFormat.mSampleRate, (int)impl->deviceFormat.mFormatID, (unsigned)impl->deviceFormat.mFormatFlags,
			   (int)impl->deviceFormat.mFramesPerPacket, (int)impl->deviceFormat.mChannelsPerFrame, (int)impl->deviceFormat.mBitsPerChannel,
			   (int)impl->deviceFormat.mBytesPerPacket, (int)impl->deviceFormat.mBytesPerFrame);
		AURenderCallbackStruct proc = { Impl::audioUnitRenderCallback, this };
		status = AudioUnitSetProperty(impl->audioUnit,
									  kAudioUnitProperty_SetRenderCallback,
#ifdef STANDALONE	/* DAS TESTING */
									  kAudioUnitScope_Global,
#else
									  kAudioUnitScope_Input,
#endif
									  kOutputBus,
									  &proc,
									  sizeof(AURenderCallbackStruct));
		if (status != noErr) {
			return appleError("Unable to set output callback", status);
		}
	}
	return 0;
}

int AppleAudioDevice::doClose()
{
	ENTER(AppleAudioDevice::doClose());
	int status = 0;
	AURenderCallbackStruct proc = { NULL, NULL };
	if (_impl->recording) {
		(void) AudioUnitSetProperty(_impl->audioUnit,
									  kAudioOutputUnitProperty_SetInputCallback,
									  kAudioUnitScope_Global,
									  kInputBus,
									  &proc,
									  sizeof(AURenderCallbackStruct));
	}
	if (_impl->playing) {
		(void) AudioUnitSetProperty(_impl->audioUnit,
									  kAudioUnitProperty_SetRenderCallback,
									  kAudioUnitScope_Global,
									  kOutputBus,
									  &proc,
									  sizeof(AURenderCallbackStruct));
	}
#ifdef STANDALONE
	if (_impl->savedDeviceSampleRate != 0.0) {
		DPRINT("AppleAudioDevice::doClose: restoring original sampling rate\n");
		setAudioHardwareRate(&_impl->savedDeviceSampleRate);
	}
#endif
	_impl->frameCount = 0;
	return status;
}

int AppleAudioDevice::doStart()
{
	ENTER(AppleAudioDevice::doStart());
	_impl->stopping = false;
    // Pre-fill the input buffers
    int preBuffers = _impl->port[!_impl->recording].audioBufFrames / _impl->port[!_impl->recording].deviceBufFrames;
    DPRINT("AppleAudioDevice::doStart: prerolling %d slices\n", preBuffers-1);
    for (int prebuf = 1; prebuf < preBuffers; ++prebuf) {
        runCallback();
	}
#if !RENDER_IN_CALLBACK
    // Start up the render thread
    _impl->startRenderThread(this);
#endif
	// Start the Audio I/O Unit
    OSStatus err = AudioOutputUnitStart(_impl->audioUnit);
	int status = (err == noErr) ? 0 : -1;
	if (status == -1)
		appleError("AppleAudioDevice::doStart: failed to start audio unit", status);
	return status;
}

int AppleAudioDevice::doPause(bool pause)
{
	_impl->paused = pause;
	return error("AppleAudioDevice: pause not yet implemented");
}

int AppleAudioDevice::doStop()
{
	ENTER(AppleAudioDevice::doStop());
#if !RENDER_IN_CALLBACK
    _impl->stopRenderThread();
#endif
	OSStatus err = AudioOutputUnitStop(_impl->audioUnit);
	int status = (err == noErr) ? 0 : -1;
	if (status == -1)
		appleError("AppleAudioDevice::doStop: failed to stop audio unit", status);
	return status;
}

#ifdef STANDALONE

//static OSStatus	DeviceListenerProc(AudioObjectID	inObjectID,
//								   UInt32			inNumberAddresses,
//								   const AudioObjectPropertyAddress    inAddresses[],
//								   void*			inClientData)
//{
//	for (UInt32 n = 0; n < inNumberAddresses; ++n) {
//		const AudioObjectPropertyAddress &addr = inAddresses[n];
//		DPRINT("DeviceListenerProc: got property change for %u\n", inAddresses[n].mSelector);
//	}
//	return noErr;
//}
//

int AppleAudioDevice::setAudioHardwareRate(double *sampleRate)
{
	UInt32 devID = 0;
	OSStatus status;
	Boolean isInput = (!_impl->playing) ? TRUE : FALSE;
	UInt32 size = sizeof(devID);
	status = AudioUnitGetProperty(_impl->audioUnit,
								  kAudioOutputUnitProperty_CurrentDevice,
								  kAudioUnitScope_Global,
								  0,	// Should not matter
								  &devID,
								  &size);
	if (status != noErr) {
		return appleError("Unable to get hardware device from audio unit", status);
	}
	
//	AudioObjectPropertyAddress addr = { kAudioDevicePropertyNominalSampleRate,
//										kAudioObjectPropertyScopeGlobal,
//										kAudioObjectPropertyElementMaster };
//	status = AudioObjectAddPropertyListener(devID, &addr, DeviceListenerProc, this);
//	if (status != noErr) {
//		return appleError("Unable to add property listener to the hardware device", status);
//	}

	double newSampleRate = *sampleRate;
	
	// Don't do any of this work if the rates do not differ!
	if (_impl->deviceFormat.mSampleRate != newSampleRate) {
		// Save original for restore when closing (good OSX software practice)
		_impl->savedDeviceSampleRate = _impl->deviceFormat.mSampleRate;

		// Test whether or not audio format property is writable.
		Boolean writeable = FALSE;
		size = sizeof(writeable);
		status = AudioDeviceGetPropertyInfo(devID,
										 0,
										 isInput,
										 kAudioDevicePropertyStreamFormat,
										 &size,
										 &writeable);
		if (status != kAudioHardwareNoError) {
			return appleError("Can't get device format writeable property", status);
		}
		if (writeable) {
			// Try to set new sample rate on hardware
			// Default all values to device's defaults then set our sample rate.
			AudioStreamBasicDescription requestedFormat;
			size = sizeof(requestedFormat);
			status = AudioDeviceGetProperty(devID,
											0,
											isInput,
											kAudioDevicePropertyStreamFormat,
											&size,
											&requestedFormat);
			if (status != kAudioHardwareNoError) {
				return appleError("Can't get device format", status);
			}
			DPRINT("Attempting to change HW sample rate from %f to %f\n", _impl->deviceFormat.mSampleRate, newSampleRate);
			requestedFormat.mSampleRate = newSampleRate;
			size = sizeof(requestedFormat);
			OSStatus err = AudioDeviceSetProperty(devID,
												  NULL,
												  0,
												  isInput,
												  kAudioDevicePropertyStreamFormat,
												  size,
												  (void *)&requestedFormat);
			switch (err) {
				case noErr:
				{
					int waitCount = 20;
					while (_impl->gotFormatNotification == false && --waitCount >= 0) {
						DPRINT("Waiting for notification from hardware...\n");
						CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);
					}
					if (waitCount >= 0) {
						DPRINT("Got it.\n");
					}
					else {
						DPRINT("Hmm.  Timed out!\n");
					}
					_impl->gotFormatNotification = false;	// reset
				}
					break;
				case kAudioDeviceUnsupportedFormatError:
					DPRINT("AudioDevice returned '%s' for sample rate %f so we wont use it\n", errToString(err), newSampleRate);
					break;
				default:
					return appleError("Can't set audio hardware format", err);
			}
            // Retrieve settings to see what we got, and compare with request.
			AudioStreamBasicDescription actualFormat;
            size = sizeof(actualFormat);
			DPRINT("Retrieving hardware format\n");
            err = AudioDeviceGetProperty(devID,
                                         0,
										 isInput,
                                         kAudioDevicePropertyStreamFormat,
                                         &size,
                                         &actualFormat);
            if (err != kAudioHardwareNoError) {
                return appleError("Can't retrieve audio hardware format", status);
            }
			if (actualFormat.mSampleRate != newSampleRate) {
				DPRINT("Unable to set sample rate -- overriding with %f\n", actualFormat.mSampleRate);
				*sampleRate = actualFormat.mSampleRate;
			}
			else {
				DPRINT("Audio hardware sample rate now set to %f\n", actualFormat.mSampleRate);
			}
			// Store this because we do not retrieve fomrmat again.
			_impl->deviceFormat.mSampleRate = actualFormat.mSampleRate;
		}
		else {
			printf("Note:  This HW's audio format is not writable\n");
			syslog(LOG_ALERT, "Note:  This HW's audio format is not writable\n");
		}
	}
	return noErr;
}

#endif	/* STANDALONE */

int AppleAudioDevice::doSetFormat(int fmt, int chans, double srate)
{
	ENTER(AppleAudioDevice::doSetFormat());
	
	_impl->bufferSampleFormat = MUS_GET_FORMAT(fmt);
	
	// Sanity check, because we do the conversion to float ourselves.
	if (_impl->bufferSampleFormat != MUS_BFLOAT && _impl->bufferSampleFormat != MUS_LFLOAT)
		return error("Only float audio buffers supported at this time.");

	OSStatus status;
#ifdef STANDALONE
	// Attempt to set hardware sample rate, and return whatever was set.
	status = setAudioHardwareRate(&srate);
	if (status != 0)
		return status;
#endif
	
	// Describe client format:  Non-interleaved floating point for both input and output
	_impl->clientFormat.mSampleRate       = srate;
	_impl->clientFormat.mFormatID         = kAudioFormatLinearPCM;
	_impl->clientFormat.mFormatFlags      = kAudioFormatFlagIsFloat|kAudioFormatFlagIsPacked|kAudioFormatFlagIsNonInterleaved;
	_impl->clientFormat.mFramesPerPacket  = 1;
	_impl->clientFormat.mChannelsPerFrame = chans;
	_impl->clientFormat.mBitsPerChannel   = 32;
	_impl->clientFormat.mBytesPerPacket   = 4;
	_impl->clientFormat.mBytesPerFrame    = 4;	// describes one noninterleaved channel
	
	UInt32 ioChannels = _impl->clientFormat.mChannelsPerFrame;
	
	if (_impl->recording) {
		Impl::Port *port = &_impl->port[REC];
		// kInputBus is the input side of the AU
		status = AudioUnitSetProperty(_impl->audioUnit,
									  kAudioUnitProperty_StreamFormat,
									  kAudioUnitScope_Output,			// client side
									  kInputBus,
									  &_impl->clientFormat,
									  sizeof(AudioStreamBasicDescription));
		if (status != noErr)
			return appleError("Cannot set input stream format", status);
		// For input, we use the channelMap just as it was created.  It will have as many
		// channels as the user has chosen to pull from the input hardware.
		if (port->channelMap != NULL) {
			if (port->channelMapCount != _impl->clientFormat.mChannelsPerFrame) {
				return error("Input channel specification does not match requested number of channels");
			}
			status = AudioUnitSetProperty(_impl->audioUnit,
										  kAudioOutputUnitProperty_ChannelMap,
										  kAudioUnitScope_Output,			// client side
										  kInputBus,
										  port->channelMap,
										  sizeof(SInt32) * port->channelMapCount);
			if (status != noErr)
				return appleError("Cannot set input channel map", status);
		}
#if DEBUG > 0
		AudioStreamBasicDescription format;
		UInt32 size = sizeof(format);
		status = AudioUnitGetProperty(_impl->audioUnit,
									  kAudioUnitProperty_StreamFormat,
									  kAudioUnitScope_Output,			// client side
									  kInputBus,
									  &format,
									  &size);
		if (status != noErr)
			return appleError("Cannot read input dev stream format", status);
		DPRINT("input client format: sr %f fmt %d flags 0x%x fpp %d cpf %d bpc %d bpp %d bpf %d\n",
			   format.mSampleRate, (int)format.mFormatID, (unsigned)format.mFormatFlags,
			   (int)format.mFramesPerPacket, (int)format.mChannelsPerFrame, (int)format.mBitsPerChannel,
			   (int)format.mBytesPerPacket, (int)format.mBytesPerFrame);
#endif
		// Always noninterleaved, so stream count == channel count.  Each stream is 1-channel
		if (port->streamCount == 0)
			port->streamCount = ioChannels;
		port->audioBufChannels = 1;
		// Always report our channel count to be what we were configured for.  We do all channel
		// matching ourselves during doGetFrames and doSendFrames.
		port->virtualChannels = chans;
		// We create a buffer equal to the internal HW channel count.
	}
	if (_impl->playing) {
		Impl::Port *port = &_impl->port[PLAY];
		status = AudioUnitSetProperty(_impl->audioUnit,
									  kAudioUnitProperty_StreamFormat,
									  kAudioUnitScope_Input,			// client side
									  kOutputBus,
									  &_impl->clientFormat,
									  sizeof(AudioStreamBasicDescription));
		if (status != noErr)
			return appleError("Cannot set output stream format", status);
		if (port->channelMap != NULL) {
			if (port->channelMapCount != _impl->clientFormat.mChannelsPerFrame) {
				return error("Output channel specification does not match requested number of channels");
			}
			// For output, the channel map needs to have as many slots as there are hardware channels.
			UInt32 hwChans = _impl->deviceFormat.mChannelsPerFrame;
			UInt32 mapSlot = 0;
			SInt32 *localMap = new SInt32[_impl->deviceFormat.mChannelsPerFrame];
			memset(localMap, 0xff, sizeof(SInt32) * hwChans);	// set all to -1
			DPRINT("output channelMap: ");
			for (UInt32 chan = 0; chan < hwChans; ++chan) {
				if (mapSlot < port->channelMapCount && port->channelMap[mapSlot] == chan) {
					localMap[chan] = mapSlot++;
				}
				DPRINT("%d, ", localMap[chan]);
			}
			DPRINT("\n");
			status = AudioUnitSetProperty(_impl->audioUnit,
										  kAudioOutputUnitProperty_ChannelMap,
										  kAudioUnitScope_Input,			// client side
										  kOutputBus,
										  localMap,
										  sizeof(SInt32) * hwChans);
			delete [] localMap;
			if (status != noErr)
				return appleError("Cannot set output channel map", status);
		}
#if DEBUG > 0
		AudioStreamBasicDescription format;
		UInt32 size = sizeof(format);
		status = AudioUnitGetProperty(_impl->audioUnit,
									  kAudioUnitProperty_StreamFormat,
									  kAudioUnitScope_Input,			// client side
									  kOutputBus,
									  &format,
									  &size);
		if (status != noErr)
			return appleError("Cannot read output dev stream format", status);
		DPRINT("output client format: sr %f fmt %d flags 0x%x fpp %d cpf %d bpc %d bpp %d bpf %d\n",
			   format.mSampleRate, (int)format.mFormatID, (unsigned)format.mFormatFlags,
			   (int)format.mFramesPerPacket, (int)format.mChannelsPerFrame, (int)format.mBitsPerChannel,
			   (int)format.mBytesPerPacket, (int)format.mBytesPerFrame);
#endif
		// Always noninterleaved, so stream count == channel count.  Each stream is 1-channel
		if (port->streamCount == 0)
			port->streamCount = ioChannels;
		port->audioBufChannels = 1;
		// Always report our channel count to be what we were configured for.  We do all channel
		// matching ourselves during doGetFrames and doSendFrames.
		port->virtualChannels = chans;
		// We create a buffer equal to the internal HW channel count.
	}
	
	int deviceFormat = NATIVE_FLOAT_FMT | MUS_NORMALIZED | MUS_NON_INTERLEAVED;
	
	setDeviceParams(deviceFormat,
					chans,
					_impl->clientFormat.mSampleRate);
	return 0;
}

int AppleAudioDevice::doSetQueueSize(int *pWriteSize, int *pCount)
{
	ENTER(AppleAudioDevice::doSetQueueSize());
	OSStatus status;
	UInt32 reqQueueFrames = *pWriteSize;
	UInt32 propSize;
#ifdef STANDALONE
	// set max buffer slice size
	status = AudioUnitSetProperty(_impl->audioUnit,
								  kAudioDevicePropertyBufferFrameSize,
								  kAudioUnitScope_Global, 0,
								  &reqQueueFrames,
								  sizeof(reqQueueFrames));
	if (status != noErr)
		return appleError("Cannot set buffer frame size on audio unit", status);

	// get value back
	UInt32 devBufferFrameSize = 0;
	propSize = sizeof(devBufferFrameSize);
	status = AudioUnitGetProperty(_impl->audioUnit,
								  kAudioDevicePropertyBufferFrameSize,
								  kAudioUnitScope_Global, 0,
								  &devBufferFrameSize,
								  &propSize);
	if (status != noErr)
		return appleError("Cannot get buffer frame size from audio unit", status);
	DPRINT("AUHal's device returned buffer frame size = %u\n", (unsigned)devBufferFrameSize);
#else	// for iOS
	UInt32 devBufferFrameSize = reqQueueFrames;
#endif
	// set the max equal to the device value so we don't ever exceed this.
	status = AudioUnitSetProperty(_impl->audioUnit,
								  kAudioUnitProperty_MaximumFramesPerSlice,
								  kAudioUnitScope_Global, 0,
								  &devBufferFrameSize,
								  sizeof(reqQueueFrames));
	if (status != noErr)
		return appleError("Cannot set max frames per slice on audio unit", status);
	// Get the property value back from audio device. We are going to use this value to allocate buffers accordingly
	UInt32 auQueueFrames = 0;
	propSize = sizeof(auQueueFrames);
	status = AudioUnitGetProperty(_impl->audioUnit,
								  kAudioUnitProperty_MaximumFramesPerSlice,
								  kAudioUnitScope_Global, 0,
								  &auQueueFrames,
								  &propSize);
	if (status != noErr)
		return appleError("Cannot get max frames per slice on audio unit", status);

	const int startDir = _impl->recording ? REC : PLAY;
	const int endDir = _impl->playing ? PLAY : REC;
	for (int dir = startDir; dir <= endDir; ++dir) {
		Impl::Port *port = &_impl->port[dir];
		port->deviceBufFrames = auQueueFrames;
		DPRINT("Device buffer length is %d frames, user req was %d frames\n",
			   (int)port->deviceBufFrames, (int)reqQueueFrames);
        port->audioBufFrames = *pCount * port->deviceBufFrames;
        
		// Notify caller of any change.
		*pWriteSize = port->audioBufFrames / *pCount;
        *pCount = port->audioBufFrames / port->deviceBufFrames;
		DPRINT("%s port buflen: %d frames. circ buffer %d frames\n",
			   dir == REC ? "Input" : "Output",
			   port->deviceBufFrames, port->audioBufFrames);
		DPRINT("\tBuffer configured for %d channels of audio\n",
			   port->audioBufChannels * port->streamCount);
		int buflen = port->audioBufFrames * port->audioBufChannels * port->streamCount;
		delete [] port->audioBuffer;
		port->audioBuffer = new float[buflen];
		if (port->audioBuffer == NULL)
			return error("Memory allocation failure for AppleAudioDevice buffer!");
		memset(port->audioBuffer, 0, sizeof(float) * buflen);
        if (dir == REC)
            port->audioBufFilled = port->audioBufFrames;    // rec buffer set to empty
		port->inLoc = 0;
		port->outLoc = 0;
	}
	if (_impl->recording) {
		_impl->inputBufferList = _impl->createInputBufferList(getDeviceChannels(), isDeviceInterleaved());
	}
	status = AudioUnitInitialize(_impl->audioUnit);
	if (status != noErr)
		return appleError("Cannot initialize the output audio unit", status);

	// Create our semaphore and signal it once so render runs first.
	_impl->renderSema = new RTSemaphore(0);
	_impl->renderSema->post();

	return 0;
}

int AppleAudioDevice::getRecordDeviceChannels() const
{
	return _impl->port[REC].virtualChannels;
}

int AppleAudioDevice::getPlaybackDeviceChannels() const
{
	return _impl->port[PLAY].virtualChannels;
}

int	AppleAudioDevice::doGetFrames(void *frameBuffer, int frameCount)
{
	const int frameChans = getFrameChannels();
	Impl::Port *port = &_impl->port[REC];
	return port->getFrames(frameBuffer, frameCount, frameChans);
}

int	AppleAudioDevice::doSendFrames(void *frameBuffer, int frameCount)
{
	const int frameChans = getFrameChannels();
	Impl::Port *port = &_impl->port[PLAY];
	int ret = port->sendFrames(frameBuffer, frameCount, frameChans);
#if DEBUG > 0
    if (_impl->underflowCount > 0) {
        --_impl->underflowCount;
        DPRINT("AppleAudioDevice::doSendFrames: underflow count -> %d -- filled now %d\n",
               _impl->underflowCount, port->audioBufFilled);
    }
#endif
    return ret;
}

int AppleAudioDevice::doGetFrameCount() const
{
	return _impl->frameCount;
}

int AppleAudioDevice::appleError(const char *msg, int status)
{
	return error(msg, errToString(status));
}

// Methods used for identification and creation

bool AppleAudioDevice::recognize(const char *desc)
{
	return true;
}

AudioDevice *AppleAudioDevice::create(const char *inputDesc, const char *outputDesc, int mode)
{
	return new AppleAudioDevice(inputDesc ? inputDesc : outputDesc);
}

static const char *
errToString(OSStatus err)
{
    const char *errstring;
    switch (err) {
    case kAudioUnitErr_InvalidProperty:
		errstring = ": Invalid Property";
		break;
    case kAudioUnitErr_InvalidParameter:
		errstring = ": Invalid Parameter";
		break;
    case kAudioUnitErr_InvalidElement:
		errstring = ": Invalid Element";
		break;
    case kAudioUnitErr_NoConnection:
		errstring = ": No Connection";
		break;
    case kAudioUnitErr_FailedInitialization:
		errstring = ": Failed Initialization";
		break;
    case kAudioUnitErr_TooManyFramesToProcess:
		errstring = ": Too Many Frames To Process";
		break;
    case kAudioUnitErr_FormatNotSupported:
		errstring = ": Format Not Supported";
		break;
	case kAudioUnitErr_PropertyNotWritable:
		errstring = ": Property Not Writable";
		break;
	case kAudioUnitErr_InvalidPropertyValue:
		errstring = ": Invalid Property Value";
		break;
	case kAudioUnitErr_PropertyNotInUse:
		errstring = ": Property Not In Use";
		break;
    case kAudioUnitErr_Uninitialized:
    case kAudioUnitErr_InvalidScope:
    case kAudioUnitErr_CannotDoInCurrentContext:
    case kAudioUnitErr_Initialized:
    case kAudioUnitErr_InvalidOfflineRender:
    case kAudioUnitErr_Unauthorized:
	case kAudioUnitErr_InvalidFile:
		errstring = ": Audio Error";
        break;
#ifdef STANDALONE
    case kAudioHardwareNoError:
        errstring = ": No error";
        break;
    case kAudioHardwareNotRunningError:
        errstring = ": Hardware not running";
        break;
    case kAudioHardwareUnspecifiedError:
        errstring = ": Unspecified error";
        break;
    case kAudioHardwareUnknownPropertyError:
        errstring = ": Unknown hardware property";
        break;
    case kAudioDeviceUnsupportedFormatError:
        errstring = ": Unsupported audio format";
        break;
    case kAudioHardwareBadPropertySizeError:
        errstring = ": Bad hardware propery size";
        break;
    case kAudioHardwareIllegalOperationError:
        errstring = ": Illegal operation";
        break;
#endif
    default:
        errstring = ": Unknown error";
    }
    return errstring;
}

#ifdef STANDALONE

static OSStatus
getDeviceList(AudioDeviceID **devList, int *devCount)
{
	UInt32 size;
	
	OSStatus err = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &size, NULL);
	if (err != kAudioHardwareNoError) {
		DERROR("Can't get hardware device list property info.\n");
		return err;
	}
	*devCount = size / sizeof(AudioDeviceID);
	*devList = new AudioDeviceID[*devCount];
	err = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &size, *devList);
	if (err != kAudioHardwareNoError) {
		DERROR("Can't get hardware device list.\n");
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
			DERROR("Cannot find default OSX device\n");
			return 0;
		}
		return devID;
	}
	for (int dev = 0; dev < devCount && devID == 0; ++dev) {
		/*
		 // In near future, switch to newer API here, like this:
		 AudioObjectPropertyAddress property_address = {
		 kAudioObjectPropertyName,                   // mSelector
		 isInput ? kAudioObjectPropertyScopeInput : kAudioObjectPropertyScopeOutput,            // mScope
		 kAudioObjectPropertyElementMaster           // mElement
		 };
		 CFStringRef theName = nil;
		 UInt32 dataSize = sizeof(property_address);
		 OSStatus err = AudioObjectGetPropertyData(devList[dev],
		 &property_address,
		 0,     // inQualifierDataSize
		 NULL,  // inQualifierData
		 &dataSize,
		 &theName);
		 */
		OSStatus err = AudioDeviceGetPropertyInfo(devList[dev],
												  0,
												  isInput,
												  kAudioDevicePropertyDeviceName,
												  &size, NULL);
		if (err != kAudioHardwareNoError) {
			DERROR("findDeviceID: Can't get device name property info for device %u\n",
					devList[dev]);
			continue;
		}
		
		char *name = new char[64 + size + 1];	// XXX Dont trust property size anymore!
		err = AudioDeviceGetProperty(devList[dev],
									 0,
									 isInput,
									 kAudioDevicePropertyDeviceName,
									 &size, name);
		if (err != kAudioHardwareNoError) {
			DERROR("findDeviceID: Can't get device name property for device %u.\n",
					(unsigned)devList[dev]);
			delete [] name;
			continue;
		}
		DPRINT("Checking device %d -- name: \"%s\"\n", dev, name);
		// For now, we must match the case as well because strcasestr() does not exist.
		if (strstr(name, devName) != NULL) {
			DPRINT("MATCH FOUND\n");
			devID = devList[dev];
		}
		delete [] name;
	}
	return devID;
}

void AppleAudioDevice::parseDeviceDescription(const char *inDesc)
{
	::getDeviceList(&_impl->deviceIDs, &_impl->deviceCount);
	if (inDesc != NULL) {
		const char *substr = strchr(inDesc, ':');
		if (substr == NULL) {
			// Descriptor is just the device name
			_impl->deviceName = new char[strlen(inDesc) + 1];
			strcpy(_impl->deviceName, inDesc);
		}
		else {
			// Extract device name
			size_t nameLen = (size_t) substr - (size_t) inDesc;
			_impl->deviceName = new char[nameLen + 1];
			strncpy(_impl->deviceName, inDesc, nameLen);
			_impl->deviceName[nameLen] = '\0';
			++substr;	// skip ':'
         	// Extract input and output stream selecters
			char *insubstr = NULL;
			const char *outsubstr = NULL;
            if ((outsubstr = strchr(substr, ',')) != NULL) {
				++outsubstr;   // skip ','
				insubstr = (char *)substr;
				insubstr[(size_t) outsubstr - (size_t) insubstr - 1] = '\0';
            }
            else {
				insubstr = (char *) (outsubstr = substr);
            }
            // Now parse stream selecters and set up channel mapping if necessary
            const char *selecters[2] = { insubstr, outsubstr };
			int chanFirst = 0, chanLast = 0;
            for (int dir = REC; dir <= PLAY; ++dir) {
				if (selecters[dir] == NULL) {
					// Do nothing;  use defaults.
				}
				else if (strchr(selecters[dir], '-') == NULL) {
					// Parse non-range selecter (single digit)
					chanFirst = (int) strtol(selecters[dir], NULL, 0);
				}
				else {
					// Parse selecter of form "X-Y"
					int idx0, idx1;
					int found = sscanf(selecters[dir], "%d-%d", &idx0, &idx1);
					if (found == 2 && idx1 >= idx0) {
						chanFirst = idx0;
						chanLast = idx1;
						_impl->port[dir].streamCount = idx1 - idx0 + 1;
					}
					else {
						DERROR("Could not parse device descriptor \"%s\"\n", inDesc);
						break;
					}
				}
				if (chanFirst != 0 || chanLast != 0) {
					int requestedChannels = (_impl->port[dir].streamCount > 0) ? _impl->port[dir].streamCount : 1;
					_impl->port[dir].channelMap = new SInt32[requestedChannels];
					_impl->port[dir].channelMapCount = requestedChannels;
					for (int n = 0; n < requestedChannels; ++n) {
						_impl->port[dir].channelMap[n] = chanFirst + n;
					}
				}
            }
			DPRINT("requested input streamCount = %d\n", _impl->port[REC].streamCount);
			if (_impl->port[REC].channelMap) {
				DPRINT("\tmapping hardware channel %d to our channel 0, etc.\n", _impl->port[REC].channelMap[0]);
			}
			DPRINT("requested output streamCount = %d\n", _impl->port[PLAY].streamCount);
 			if (_impl->port[PLAY].channelMap) {
				DPRINT("\tmapping our channel 0 to hardware channel %d, etc.\n", _impl->port[PLAY].channelMap[0]);
			}
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
}
#else
void AppleAudioDevice::parseDeviceDescription(const char *inDesc)
{
	
}
#endif

