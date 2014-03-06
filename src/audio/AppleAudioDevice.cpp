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
#ifndef IOS
#include <CoreAudio/CoreAudio.h>
#include <CoreAudio/AudioHardware.h>
#endif
// Test code to see if it is useful to make sure HW and render threads do not
// access the circular buffer at the same time.  Does not seem to be necessary.
#define LOCK_IO 1

#if LOCK_IO
#include <Lockable.h>
#endif

#include <syslog.h>
#define DEBUG 0

#if DEBUG > 0
#define ENTER(fun) printf("Entering " #fun "\n");
#else
#define ENTER(fun)
#endif

inline int max(int x, int y) { return (x >= y) ? x : y; }
inline int min(int x, int y) { return (x < y) ? x : y; }

const int kOutputBus = 0;
const int kInputBus = 1;

static const int REC = 0, PLAY = 1;

#ifndef IOS
static AudioDeviceID findDeviceID(const char *devName, AudioDeviceID *devList,
								  int devCount, Boolean isInput);
#endif

//static const char *errToString(OSStatus err);

struct AppleAudioDevice::Impl {
    Impl();
    ~Impl();

#ifndef IOS
	AudioDeviceID			*deviceIDs;				// Queried on system.
	int						deviceCount;			// Queried on system.
	char 					*deviceName;			// Passed in by user.
	AudioDeviceID			deviceID;
#endif
	struct Port {
		int						streamIndex;		// Which stream
		int						streamCount;		// How many streams open
		int						streamChannel;		// 1st chan of first stream
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
	AudioStreamBasicDescription deviceFormat;	// format
	int						bufferSampleFormat;
	int						frameCount;
	bool					paused;
	bool					stopping;
	bool					recording;				// Used by OSX code
	bool					playing;				// Used by OSX code
	pthread_t               renderThread;
    RTSemaphore *           renderSema;
    int                     underflowCount;
	AudioUnit				audioUnit;

    int                     startRenderThread(AppleAudioDevice *parent);
    void                    stopRenderThread();
	inline int				outputDeviceChannels() const;
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
#if DEBUG > 0
	printf("AppleAudioDevice::doGetFrames: inFrameCount = %d REC filled = %d\n", inFrameCount, audioBufFilled);
#endif
#if LOCK_IO
    lock.lock();
#endif
	int streamsToCopy = 0;
#ifndef IOS
	assert(inFrameCount <= audioBufFilled);
#else
	if (inFrameCount <= audioBufFilled)		// For IOS, we need to handle the case where there is no audio to pull.
#endif
		streamsToCopy = inFrameChans < streamCount ? inFrameChans : streamCount;
	
#if DEBUG > 0
	printf("Copying %d non-interleaved internal bufs into %d-channel user frame\n",
		   streamsToCopy, inFrameChans);
#endif
	int stream;
	for (stream = 0; stream < streamsToCopy; ++stream) {
		// Offset into serially-ordered, multi-channel non-interleaved buf.
		register float *buf = &audioBuffer[stream * bufFrames];
		float *frame = fFrameBuffer[stream];
		bufLoc = outLoc;
#if DEBUG > 1
		printf("\tstream %d: raw offset into mono internal buffer: %d (%d * %d)\n", stream, buf - &audioBuffer[0], stream, bufFrames);
		printf("\tread internal (already-offset) buf starting at outLoc %d\n", bufLoc);
#endif
		// Write each monaural frame from circ. buffer into a non-interleaved output frame.
		for (int out=0; out < inFrameCount; ++out) {
			if (bufLoc >= bufFrames)
				bufLoc -= bufFrames;	// wrap
			frame[out] = buf[bufLoc++];
		}
	}
	// Zero out any remaining frame channels
	for (; stream < inFrameChans; ++stream) {
#if DEBUG > 0
		printf("Zeroing user frame channel %d\n", stream);
#endif
		memset(fFrameBuffer[stream], 0, inFrameCount * sizeof(float));
	}
	outLoc = bufLoc;
	audioBufFilled -= inFrameCount;
#if LOCK_IO
    lock.unlock();
#endif
#if DEBUG > 1
	printf("\tREC Filled now %d\n", audioBufFilled);
#endif
#if DEBUG > 1
	printf("\tREC bufLoc ended at %d. Returning frameCount = %d\n", bufLoc, inFrameCount);
#endif
	return inFrameCount;
}

int
AppleAudioDevice::Impl::Port::sendFrames(void *frameBuffer, int frameCount, int frameChans)
{
	float **fFrameBuffer = (float **) frameBuffer;		// non-interleaved
	const int bufFrames = audioBufFrames;
	int loc = 0;
#if DEBUG > 0
	printf("AppleAudioDevice::doSendFrames: frameCount = %d, PLAY filled = %d\n", frameCount, audioBufFilled);
#endif
#if DEBUG > 0
	printf("Copying %d channel user frame into %d non-interleaved internal buf channels\n", frameChans, streamCount);
#endif
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
#if DEBUG > 0
			printf("Zeroing internal buf channel %d\n", stream);
#endif
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
#if DEBUG > 1
	printf("\tPLAY Filled now %d\n", audioBufFilled);
#endif
#if DEBUG > 1
	printf("\tPLAY inLoc ended at %d. Returning frameCount = %d\n", inLoc, frameCount);
#endif
	return frameCount;
}

AppleAudioDevice::Impl::Impl()
:
#if defined(OSX)
deviceIDs(NULL), deviceName(NULL), deviceID(0),
#endif
renderThread(NULL), renderSema(NULL), underflowCount(0)
{
	paused = false;
	stopping = false;
	recording = false;
	playing = false;
	for (int n = REC; n <= PLAY; ++n) {
		port[n].streamIndex = 0;
		port[n].streamCount = 0;		// Indicates that we are using the default.
		port[n].streamChannel = 0;
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
    delete [] (char *) port[REC].streamDesc;
	delete [] port[REC].audioBuffer;
	delete [] (char *) port[PLAY].streamDesc;
	delete [] port[PLAY].audioBuffer;
#if defined(OSX)
	delete [] deviceIDs;
	delete [] deviceName;
#endif
    delete renderSema;
}

// Utilities

inline int inAvailable(int filled, int size) {
	return size - filled;
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
#if DEBUG > 0
        printf("AppleAudioDevice::Impl::renderProcess waiting...\n");
#endif
        impl->renderSema->wait();
        if (impl->stopping) {
#if DEBUG > 0
            printf("AppleAudioDevice::Impl::renderProcess woke to stop -- breaking out\n");
#endif
            break;
        }
#if DEBUG > 0
        if (impl->underflowCount > 0) {
            printf("\nAppleAudioDevice::Impl::renderProcess woke up -- underflow count %d -- running slice\n", impl->underflowCount);
            --impl->underflowCount;
        }
		else {
			printf("\nAppleAudioDevice::Impl::renderProcess woke up -- running slice\n");
		}
#endif
        bool ret = device->runCallback();
        if (ret == false) {
#if DEBUG > 0
			printf("AppleAudioDevice: renderProcess: run callback returned false -- breaking out\n");
#endif
            break;
        }
    }
#if DEBUG > 0
    printf("AppleAudioDevice: renderProcess: calling stop callback\n");
#endif
    device->stopCallback();
#if DEBUG > 0
    printf("AppleAudioDevice: renderProcess exiting\n");
#endif
	return NULL;
}

OSStatus AppleAudioDevice::Impl::audioUnitRenderCallback(void *inUserData,
												AudioUnitRenderActionFlags *ioActionFlags,
												const AudioTimeStamp *inTimeStamp,
												UInt32 inBusNumber,
												UInt32 inNumberFrames,
												AudioBufferList *ioData)
{
	Impl *impl = (Impl *) inUserData;
	OSStatus result = noErr;
	int framesAdvanced = 0;
	Port *port;
#if DEBUG > 0
	printf("\nAppleAudioDevice: top of audioUnitRenderCallback\n");
#endif
	if (impl->recording) {
		// Pull audio from hardware into ioData, then copy to our buffer
		result = AudioUnitRender(impl->audioUnit, ioActionFlags, inTimeStamp, kInputBus, inNumberFrames, ioData);
		if (result != noErr)
			return result;
		port = &impl->port[REC];
#if LOCK_IO
        if (!port->lock.tryLock()) {
#if DEBUG > 0
            printf("AppleAudioDevice: record section skipped due to block on render thread\n");
#endif
            goto Play;
        }
#endif
		const int destchans = port->audioBufChannels;
		// Length in samples, not frames.
		const int bufLen = port->audioBufFrames * destchans;
		// How many frames are available from HW.
		const int framesToRead = inNumberFrames;
		// How many frames' space are available in our buffer.
		int frameSpaceAvail = ::inAvailable(port->audioBufFilled, port->audioBufFrames);
		
#if DEBUG > 0
		printf("AppleAudioDevice: record section (in buffer %d)\n", port->streamIndex);
		printf("framesToRead = %d, frameSpaceAvail = %d, Filled = %d\n", framesToRead, frameSpaceAvail, port->audioBufFilled);
#endif

		// Check for enough space to copy audio from HW.
		if (frameSpaceAvail < framesToRead) {
#if !defined(IOS)	// IRTCMIX starts recording from the input before inTraverse() starts pulling the recorded audio
#if DEBUG > 0
			printf("AppleAudioDevice (record): frameSpaceAvail (%d) less than needed -- OVERFLOW\n", frameSpaceAvail);
#endif
			impl->renderSema->post();
			frameSpaceAvail = ::inAvailable(port->audioBufFilled, port->audioBufFrames);
#if DEBUG > 0
			printf("\tafter posting to render thread, frameSpaceAvail = %d\n", frameSpaceAvail);
#endif
#else	// IOS
#if DEBUG > 0
			printf("AppleAudioDevice (record): frameSpaceAvail (%d) less than needed -- overwriting\n", frameSpaceAvail);
#endif
			port->audioBufFilled = 0;
			frameSpaceAvail = port->audioBufFrames;
#endif	// !IOS
		}

#if DEBUG > 1
        printf("\tREC inLoc begins at %d (out of %d)\n", port->inLoc, bufLen);
#endif
        int	framesCopied = 0;
        // Write recorded audio data from audio unit into port->audioBuffer.
        //   Treat it as circular buffer.  Channel counts always match here.
        while (framesCopied < framesToRead) {
            const int srcchans = 1;		// non-interleaved input - change this if we use interleaved
            int inLoc = port->inLoc;
            for (int stream = 0; stream < port->streamCount; ++stream) {
                inLoc = port->inLoc;
                const int strIdx = stream + port->streamIndex;
                register float *src = (float *) ioData->mBuffers[strIdx].mData;
                register float *dest = &port->audioBuffer[stream * port->audioBufFrames];
#if DEBUG > 1
                printf("\tstream %d: copying from HW buffer %d (%p) into internal buf %p at raw offset %d (%d * %d)\n",
                       stream, strIdx, src, dest, dest - &port->audioBuffer[0], stream, port->audioBufFrames);
				//                printf("\t incrementing HW buffer pointer by %d for each frame\n", srcchans);
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
#if LOCK_IO
        port->lock.unlock();
#endif
#if DEBUG > 0
        printf("\tREC Filled = %d\n", port->audioBufFilled);
#endif
#if DEBUG > 1
        printf("\tREC inLoc ended at %d\n", port->inLoc);
#endif
	}
Play:
	if (impl->playing) {
		// Copy audio from our rendered buffer(s) into ioData to be written to the hardware
		port = &impl->port[PLAY];
#if LOCK_IO
        if (!port->lock.tryLock()) {
#if DEBUG > 0
            printf("AppleAudioDevice: playback section skipped due to block on render thread\n");
#endif
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
		
#if DEBUG > 0
		printf("AppleAudioDevice: playback section (out buffer %d)\n", port->streamIndex);
		printf("framesAvail (Filled) = %d\n", framesAvail);
#endif
		if (framesAvail < framesToWrite) {
#if DEBUG > 0
            if ((impl->underflowCount %4) == 0) {
                fprintf(stderr, "AppleAudioDevice (playback): framesAvail (%d) < needed (%d) -- UNDERFLOW\n", framesAvail, framesToWrite);
                printf("\tzeroing input buffer and going on\n");
            }
            ++impl->underflowCount;
#endif
            for (int stream = 0; stream < port->streamCount; ++stream) {
                memset(&port->audioBuffer[stream * port->audioBufFrames], 0, bufLen * sizeof(float));
            }
            framesAvail = framesToWrite;
            port->audioBufFilled += framesToWrite;
		}
#if DEBUG > 1
        printf("\tPLAY outLoc begins at %d (out of %d)\n",
               port->outLoc, bufLen);
#endif
        int framesDone = 0;
        // Audio data has been written into port->audioBuffer during doSendFrames.
        //   Treat it as circular buffer.
        // Copy that audio into the ioData buffer pointers.  Channel counts always match.
        while (framesDone < framesToWrite) {
            int bufLoc = port->outLoc;
#if DEBUG > 0
            printf("\tLooping for %d (%d-channel) stream%s\n",
                   port->streamCount, destchans, port->streamCount > 1 ? "s" : "");
#endif
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
#if DEBUG > 0
        printf("\tPLAY Filled = %d\n", port->audioBufFilled);
#endif
#if DEBUG > 1
        printf("\tPLAY bufLoc ended at %d\n", port->outLoc);
#endif
	}
	impl->frameCount += framesAdvanced;
#if DEBUG > 0
	printf("\tWaking render thread\n");
#endif
    impl->renderSema->post();
End:
#if DEBUG > 0
	printf("AppleAudioDevice: leaving audioUnitRenderCallback\n\n");
#endif
	return noErr;
}

void AppleAudioDevice::Impl::propertyListenerProc(void *inRefCon,
											 AudioUnit inUnit,
											 AudioUnitPropertyID inID,
											 AudioUnitScope inScope,
											 AudioUnitElement inElement)
{
	printf("propertyListenerProc called for prop %d, scope %d\n",
		   (int)inID, (int)inScope);
}

int AppleAudioDevice::Impl::startRenderThread(AppleAudioDevice *parent)
{
#if DEBUG > 0
	printf("\tAppleAudioDevice::Impl::startRenderThread: starting thread\n");
#endif
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int status = pthread_attr_setschedpolicy(&attr, SCHED_RR);
	if (status != 0) {
		fprintf(stderr, "startRenderThread: Failed to set scheduling policy\n");
	}
	status = pthread_create(&renderThread, &attr, renderProcess, parent);
	pthread_attr_destroy(&attr);
	if (status < 0) {
		fprintf(stderr, "Failed to create render thread");
	}
	return status;
}

void AppleAudioDevice::Impl::stopRenderThread()
{
    assert(renderThread != 0);	// should not get called again!
	stopping = true;
#if DEBUG > 0
    printf("AppleAudioDevice::Impl::stopRenderThread: posting to semaphore for thread\n");
#endif
    renderSema->post();  // wake up, it's time to die
#if DEBUG > 0
    printf("AppleAudioDevice::Impl::stopRenderThread: waiting for thread to finish\n");
#endif
    if (pthread_join(renderThread, NULL) == -1) {
        fprintf(stderr, "AppleAudioDevice::Impl::stopRenderThread: terminating thread!\n");
        pthread_cancel(renderThread);
        renderThread = 0;
    }
#if DEBUG > 0
    printf("\tAppleAudioDevice::Impl::stopRenderThread: thread done\n");
#endif
}

// Main class implementation

AppleAudioDevice::AppleAudioDevice(const char *desc) : _impl(new Impl)
{
	parseDeviceDescription(desc);
}

AppleAudioDevice::~AppleAudioDevice()
{
	//printf("AppleAudioDevice::~AppleAudioDevice()\n");
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
	impl->recording = ((mode & Record) != 0);
	impl->playing = ((mode & Playback) != 0);

	// Describe audio component
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
#ifndef IOS
	desc.componentSubType = kAudioUnitSubType_HALOutput;	// OSX
#else
	desc.componentSubType = kAudioUnitSubType_RemoteIO;		// iOS
#endif
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	
	AudioComponent comp = AudioComponentFindNext(NULL, &desc);
	status = AudioComponentInstanceNew(comp, &impl->audioUnit);
	if (status != noErr) {
		return error("Unable to create the output audio unit");
	}
#ifndef IOS
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
		return error("Unable to set hardware device on audio unit");
	}
#endif
	// Enable IO for playback and/or record
	
	/* NOTE:  MUST ONLY BE ENABLED ON THE HW SIDE (Output Scope, Output Element) */
	UInt32 enableInput = impl->recording;
	UInt32 enableOutput = impl->playing;
	
	status = AudioUnitSetProperty(impl->audioUnit,
								  kAudioOutputUnitProperty_EnableIO,
								  kAudioUnitScope_Input,
								  kInputBus,
								  &enableInput,
								  sizeof(enableInput));

	if (status != noErr) {
		return error("Unable to Enable/Disable input I/O on audio unit");
	}
	status = AudioUnitSetProperty(impl->audioUnit,
								  kAudioOutputUnitProperty_EnableIO,
								  kAudioUnitScope_Output,
								  kOutputBus,
								  &enableOutput,
								  sizeof(enableOutput));
	if (status != noErr) {
		return error("Unable to Enable/Disable output I/O on audio unit");
	}

	status = AudioUnitAddPropertyListener(impl->audioUnit, kAudioUnitProperty_SampleRate, Impl::propertyListenerProc, impl);

	if (status != noErr) {
		return error("Unable to set listener on audio unit");
	}
	
	// Set I/O callback
	AURenderCallbackStruct proc = { Impl::audioUnitRenderCallback, impl };
	if (impl->recording) {
		status = AudioUnitSetProperty(impl->audioUnit,
									  kAudioUnitProperty_SetRenderCallback,
									  kAudioUnitScope_Global,
									  kInputBus,
									  &proc,
									  sizeof(AURenderCallbackStruct));
		if (status != noErr) {
			return error("Unable to set input callback");
		}
	}
	if (impl->playing) {
		status = AudioUnitSetProperty(impl->audioUnit,
									  kAudioUnitProperty_SetRenderCallback,
									  kAudioUnitScope_Global,
									  kOutputBus,
									  &proc,
									  sizeof(AURenderCallbackStruct));
		if (status != noErr) {
			return error("Unable to set output callback");
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
									  kAudioUnitProperty_SetRenderCallback,
									  kAudioUnitScope_Input,
									  kInputBus,
									  &proc,
									  sizeof(AURenderCallbackStruct));
	}
	if (_impl->playing) {
		(void) AudioUnitSetProperty(_impl->audioUnit,
									  kAudioUnitProperty_SetRenderCallback,
									  kAudioUnitScope_Output,
									  kOutputBus,
									  &proc,
									  sizeof(AURenderCallbackStruct));
	}
	_impl->frameCount = 0;
	return status;
}

int AppleAudioDevice::doStart()
{
	ENTER(AppleAudioDevice::doStart());
	_impl->stopping = false;
    // Pre-fill the input buffers
    int preBuffers =  _impl->port[!_impl->recording].audioBufFrames / _impl->port[!_impl->recording].deviceBufFrames - 1;
#if DEBUG > 0
    printf("AppleAudioDevice::doStart: prerolling %d slices\n", preBuffers);
#endif
    for (int prebuf = 1; prebuf < preBuffers; ++prebuf) {
        runCallback();
	}
    // Start up the render thread
    _impl->startRenderThread(this);
	// Start the Audio I/O Unit
    OSStatus err = AudioOutputUnitStart(_impl->audioUnit);
	int status = (err == noErr) ? 0 : -1;
	if (status == -1)
		error("AppleAudioDevice::doStart: failed to start audio unit");
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
    _impl->stopRenderThread();
	OSStatus err = AudioOutputUnitStop(_impl->audioUnit);
	int status = (err == noErr) ? 0 : -1;
	if (status == -1)
		error("AppleAudioDevice::doStop: failed to stop audio unit");
	return status;
}


int AppleAudioDevice::doSetFormat(int fmt, int chans, double srate)
{
	ENTER(AppleAudioDevice::doSetFormat());
	
	_impl->bufferSampleFormat = MUS_GET_FORMAT(fmt);
	
	// Sanity check, because we do the conversion to float ourselves.
	if (_impl->bufferSampleFormat != MUS_BFLOAT && _impl->bufferSampleFormat != MUS_LFLOAT)
		return error("Only float audio buffers supported at this time.");
	
	// Describe format:  Non-interleaved floating point for both input and output
	_impl->deviceFormat.mSampleRate       = srate;
	_impl->deviceFormat.mFormatID         = kAudioFormatLinearPCM;
	_impl->deviceFormat.mFormatFlags      = kAudioFormatFlagIsFloat|kAudioFormatFlagIsPacked|kAudioFormatFlagIsNonInterleaved;
	_impl->deviceFormat.mFramesPerPacket  = 1;
	_impl->deviceFormat.mChannelsPerFrame = chans;
	_impl->deviceFormat.mBitsPerChannel   = 32;
	_impl->deviceFormat.mBytesPerPacket   = 4;
	_impl->deviceFormat.mBytesPerFrame    = 4;	// describes one noninterleaved channel
	
	// NOTE:  For now, we force the HW to the number of channels we request.
	// Later, we will allow user to configure output to any consecutive set
	// of streams on input and/or output.
	UInt32 ioChannels = _impl->deviceFormat.mChannelsPerFrame;
	
	OSStatus status;
	if (_impl->recording) {
		status = AudioUnitSetProperty(_impl->audioUnit,
									  kAudioUnitProperty_StreamFormat,
									  kAudioUnitScope_Output,
									  kInputBus,
									  &_impl->deviceFormat,
									  sizeof(AudioStreamBasicDescription));
		if (status != noErr)
			return error("Cannot set input stream format");
		Impl::Port *port = &_impl->port[REC];
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
		status = AudioUnitSetProperty(_impl->audioUnit,
									  kAudioUnitProperty_StreamFormat,
									  kAudioUnitScope_Input,
									  kOutputBus,
									  &_impl->deviceFormat,
									  sizeof(AudioStreamBasicDescription));
		if (status != noErr)
			return error("Cannot set output stream format");
		Impl::Port *port = &_impl->port[PLAY];
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
					_impl->deviceFormat.mSampleRate);
	return 0;
}

int AppleAudioDevice::doSetQueueSize(int *pWriteSize, int *pCount)
{
	ENTER(AppleAudioDevice::doSetQueueSize());
	OSStatus status;
	UInt32 reqQueueFrames = *pWriteSize;
	UInt32 propSize;
#ifndef IOS
	// set max buffer slice size
	status = AudioUnitSetProperty(_impl->audioUnit,
								  kAudioDevicePropertyBufferFrameSize,
								  kAudioUnitScope_Global, 0,
								  &reqQueueFrames,
								  sizeof(reqQueueFrames));
	if (status != noErr)
		return error("Cannot set buffer frame size on audio unit");

	// get value back
	UInt32 devBufferFrameSize = 0;
	propSize = sizeof(devBufferFrameSize);
	status = AudioUnitGetProperty(_impl->audioUnit,
								  kAudioDevicePropertyBufferFrameSize,
								  kAudioUnitScope_Global, 0,
								  &devBufferFrameSize,
								  &propSize);
	if (status != noErr)
		return error("Cannot get buffer frame size from audio unit");
#if DEBUG > 0
	printf("AUHal's device returned buffer frame size = %u\n", (unsigned)devBufferFrameSize);
#endif
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
		return error("Cannot set max frames per slice on audio unit");
	// Get the property value back from audio device. We are going to use this value to allocate buffers accordingly
	UInt32 auQueueFrames = 0;
	propSize = sizeof(auQueueFrames);
	status = AudioUnitGetProperty(_impl->audioUnit,
								  kAudioUnitProperty_MaximumFramesPerSlice,
								  kAudioUnitScope_Global, 0,
								  &auQueueFrames,
								  &propSize);
	if (status != noErr)
		return error("Cannot get max frames per slice on audio unit");

	const int startDir = _impl->recording ? REC : PLAY;
	const int endDir = _impl->playing ? PLAY : REC;
	for (int dir = startDir; dir <= endDir; ++dir) {
		Impl::Port *port = &_impl->port[dir];
		port->deviceBufFrames = auQueueFrames;
#if DEBUG > 0
		printf("Device buffer length is %d frames, user req was %d frames\n",
			   (int)port->deviceBufFrames, (int)reqQueueFrames);
#endif
        port->audioBufFrames = *pCount * port->deviceBufFrames;
        
		// Notify caller of any change.
		*pWriteSize = port->audioBufFrames / *pCount;
        *pCount = port->audioBufFrames / port->deviceBufFrames;
#if DEBUG > 0
		printf("%s device buflen: %d frames. circ buffer %d frames\n",
			   dir == REC ? "Input" : "Output",
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
        if (dir == REC)
            port->audioBufFilled = port->audioBufFrames;    // rec buffer set to empty
		port->inLoc = 0;
		port->outLoc = 0;
	}
	
	status = AudioUnitInitialize(_impl->audioUnit);
	if (status != noErr)
		return error("Cannot initialize the output audio unit");

	// Create our semaphore
	_impl->renderSema = new RTSemaphore(1);

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
        printf("AppleAudioDevice::Impl::doSendFrames: underflow count -> %d -- filled now %d\n",
               _impl->underflowCount, port->audioBufFilled);
    }
#endif
    return ret;
}

int AppleAudioDevice::doGetFrameCount() const
{
	return _impl->frameCount;
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

#ifndef IOS

static OSStatus
getDeviceList(AudioDeviceID **devList, int *devCount)
{
	UInt32 size;
	
	OSStatus err = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &size, NULL);
	if (err != kAudioHardwareNoError) {
		fprintf(stderr, "Can't get hardware device list property info.\n");
		return err;
	}
	*devCount = size / sizeof(AudioDeviceID);
	*devList = new AudioDeviceID[*devCount];
	err = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &size, *devList);
	if (err != kAudioHardwareNoError) {
		fprintf(stderr, "Can't get hardware device list.\n");
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
			fprintf(stderr, "Cannot find default OSX device\n");
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
			fprintf(stderr, "findDeviceID: Can't get device name property info for device %u\n",
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
			fprintf(stderr, "findDeviceID: Can't get device name property for device %u.\n",
					(unsigned)devList[dev]);
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

void AppleAudioDevice::parseDeviceDescription(const char *inDesc)
{
	::getDeviceList(&_impl->deviceIDs, &_impl->deviceCount);
	if (inDesc != NULL) {
		char *substr = strchr(inDesc, ':');
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
						fprintf(stderr, "Could not parse device descriptor \"%s\"\n", inDesc);
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
}
#else
void AppleAudioDevice::parseDeviceDescription(const char *inDesc)
{
	
}
#endif

