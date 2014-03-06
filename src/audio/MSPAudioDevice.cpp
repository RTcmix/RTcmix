//
//  MSPAudioDevice.cpp
//  RTcmix
//
//  Created by Douglas Scott on 12/2/13.
//
//

#include "MSPAudioDevice.h"
#include <ext.h>
#include <z_dsp.h>
#include <ext_strings.h>
#include <edit.h>
#include <ext_wind.h>
#include <ext_obex.h>
#include <string.h>

#ifndef MAX_INPUTS
#define MAX_INPUTS 128
#endif

#ifndef MAX_OUTPUTS
#define MAX_OUTPUTS 32
#endif

struct MSP_Info
{
	MSP_Info() : mDspArgs(NULL), mInputConnected(NULL), mDisabled(NULL) {}
	void **mDspArgs;		// list of arguments to be handed to perform method
	short *mInputConnected;	// pointer to array from rtcmix~.c
	long *mDisabled;		// pointer to variable inside Max object indicating mute status
};

extern void *gMSPAudioState;	// defined in main.cpp

struct MSPAudioDevice::Impl
{
	Impl(MSPAudioDevice *inParent, int numAudioInputs, int numControlInputs, int numAudioOutputs);
	~Impl();
	int					configure(MSP_Info &inInfo);
	bool				inputIsEnabled(int input) const { return mInfo.mInputConnected[input] != 0; }
	bool				isMuted() const { return *mInfo.mDisabled != 0; }

	MSPAudioDevice *	mParent;
	int					mNumAudioInputs;
	int					mNumOtherInputs;
	int					mNumAudioOutputs;
	int					mFrameSize;
	float *				mInputBuffers[MAX_INPUTS];
	float *				mOutputBuffers[MAX_OUTPUTS];
	int					mFrameCount;
	MSP_Info			mInfo;
	
	static t_int *		msp_Perform(t_int *);
};

MSPAudioDevice::Impl::Impl(MSPAudioDevice *inParent,
						   int numAudioInputs, int numControlInputs, int numAudioOutputs)
	: mParent(inParent),
	  mNumAudioInputs(numAudioInputs), mNumOtherInputs(numControlInputs), mNumAudioOutputs(numAudioOutputs),
	  mFrameSize(0), mFrameCount(0)
{
	int n;
	for (n=0; n<MAX_INPUTS; ++n) mInputBuffers[n] = NULL;
	for (n=0; n<MAX_OUTPUTS; ++n) mOutputBuffers[n] = NULL;
}

MSPAudioDevice::Impl::~Impl()
{
	
}

int	MSPAudioDevice::Impl::configure(MSP_Info &inInfo)
{
	mInfo = inInfo;
	int totalInputs = mNumAudioInputs + mNumOtherInputs;
	mInfo.mDspArgs[0] = this;
//	cpost("MSPAudioDevice::Impl::configure: calling dsp_addv(%p, %d, %p)", msp_Perform, (totalInputs + mNumAudioOutputs + 2), mInfo.mDspArgs);
	dsp_addv(msp_Perform, (totalInputs + mNumAudioOutputs + 2), mInfo.mDspArgs); //add them to the signal chain
	return 0;
}

t_int *
MSPAudioDevice::Impl::msp_Perform(t_int *args)
{
	// args: ?, context, input0, ..., inputX, pinlet0, ..., pinletY, output0, ..., outputZ, vec_size
	MSPAudioDevice::Impl *impl = (MSPAudioDevice::Impl *)(args[1]);
//	cpost("msp_Perform: audio inputs: %d pinputs: %d outputs: %d\n", impl->mNumAudioInputs, impl->mNumOtherInputs, impl->mNumAudioOutputs);
	int totalInputs = impl->mNumAudioInputs + impl->mNumOtherInputs;
	int totalOutputs = impl->mNumAudioOutputs;
	int n;
//	cpost("msp_Perform: nframes: %d\n", nframes);
	//	Check to see if we should skip this routine if the patcher is "muted".
	//	I also setup of "power" messages for expensive objects, so that the
	//	object itself can be turned off directly. this can be convenient sometimes.
	//	in any case, all audio objects should have this
	if (impl->isMuted() || !impl->mParent->isRunning()) {
		goto done;
	}
	//check to see if we have a signal or float message connected to input.  NULL inputs will be ignored
	for (n = 0; n < totalInputs; n++) {
		impl->mInputBuffers[n] = impl->inputIsEnabled(n) ? (float *)args[n+2] : NULL;
	}
	for (n = 0; n < totalOutputs; n++) {
		impl->mOutputBuffers[n] = (float *)args[totalInputs+n+2];
	}
	impl->mParent->runCallback();
done:
	return args + totalInputs + totalOutputs + 3;
}

// Device description: "MSP:inaudiochans,inextrachans,outaudiochans,
MSPAudioDevice::MSPAudioDevice(const char *inDesc)
{
	if (inDesc != NULL) {
		int inChans = 0, pInChans = 0, outChans = 0;
		if (sscanf(inDesc, "MSP: %d, %d, %d", &inChans, &pInChans, &outChans) != 3) {
			fprintf(stderr, "Bad MSPAudioDevice specification: '%s'\n", inDesc);
			throw -1;
		}
		_impl = new Impl(this, inChans, pInChans, outChans);
	}
}

MSPAudioDevice::~MSPAudioDevice()
{
	close();
	delete _impl;
}

// Recognizer
bool	MSPAudioDevice::recognize(const char *desc)
{
	return (desc != NULL) && strncmp(desc, "MSP:", 4) == 0;
}

// Creator
AudioDevice*	MSPAudioDevice::create(const char *inputDesc, const char *outputDesc, int mode)
{
	AudioDevice *dev = NULL;
	try {
		dev = new MSPAudioDevice(inputDesc ? inputDesc : outputDesc);
	}
	catch(...) {
		
	}
	return dev;
}

int MSPAudioDevice::doOpen(int mode)
{
	return 0;
}

int MSPAudioDevice::doClose()
{
	_impl->mFrameCount = 0;
	return 0;
}

int MSPAudioDevice::doStart()
{
	if (gMSPAudioState == NULL) {
		return error("MSP dsp state is NULL");
	}
	MSP_Info *pInfo = (MSP_Info *) gMSPAudioState;
	return _impl->configure(*pInfo);
}

int MSPAudioDevice::doStop()
{
	_impl->mFrameCount = 0;
	return 0;
}

int MSPAudioDevice::doPause(bool)
{
	return 0;
}

int MSPAudioDevice::doSetFormat(int sampfmt, int chans, double srate)
{
	int format = MUS_GET_FORMAT(sampfmt);
	
	// Sanity check, because we do the conversion to float ourselves.
	if (format != MUS_BFLOAT && format != MUS_LFLOAT)
		return error("Only float audio buffers supported at this time.");

	if (chans != _impl->mNumAudioOutputs)
		return error("Channel mismatch between requested and original initialization");
	
	int deviceFormat = NATIVE_FLOAT_FMT | MUS_NON_INTERLEAVED | MUS_NORMALIZED;

	setDeviceParams(deviceFormat, chans, srate);
	return 0;
}

int MSPAudioDevice::doSetQueueSize(int *pWriteSize, int *pCount)
{
	_impl->mFrameSize = *pWriteSize;
	*pCount = 1;
	return 0;
}

int MSPAudioDevice::doGetFrameCount() const
{
	return _impl->mFrameCount;
}

int	MSPAudioDevice::doGetFrames(void *outFrameBuffer, int frameCount)
{
	float **outBuffers = (float **) outFrameBuffer;
	const int channels = _impl->mNumAudioOutputs;
	for (int chan = 0; chan < channels; ++chan) {
		float *in = _impl->mInputBuffers[chan];
		float *out = outBuffers[chan];
		if (in) {
			for (int n = 0; n < frameCount; ++n)
				out[n] = in[n];
		}
		else {	// disabled input
			for (int n = 0; n < frameCount; ++n)
				out[n] = 0.0f;
		}
	}
	return frameCount;
}

int	MSPAudioDevice::doSendFrames(void *inFrameBuffer, int frameCount)
{
	float **inBuffers = (float **) inFrameBuffer;
	const int channels = _impl->mNumAudioOutputs;
	for (int chan = 0; chan < channels; ++chan) {
		float *in = inBuffers[chan];
		float *out = _impl->mOutputBuffers[chan];
		if (!_impl->isMuted()) {
			for (int n = 0; n < frameCount; ++n)
				out[n] = in[n];
		}
		else {
			// Muted object
			for (int n = 0; n < frameCount; ++n)
				out[n] = 0.0f;
		}
	}
	_impl->mFrameCount += frameCount;
	return frameCount;
}
