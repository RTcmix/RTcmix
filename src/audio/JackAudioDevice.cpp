// JackAudioDevice.cpp

#if defined(JACK)

#include "JackAudioDevice.h"
#include <jack/jack.h>

// Here is a partial list of base class helper methods you may use to check
// the state of the system.  Not all of these have valid values at all times --
// some return state that you set via the init/open sequence.  Most should be
// self-explanatory:

// bool		isOpen() const;
// bool		isRunning() const;				-- has start() been called
// bool		isPaused() const;
// int		getFrameFormat() const;			-- MUS_BSHORT, etc.
// int		getDeviceFormat() const;
// bool		isFrameFmtNormalized() const;
// bool		isDeviceFmtNormalized() const;
// bool		isFrameInterleaved() const;
// bool		isDeviceInterleaved() const;
// int		getFrameChannels() const;
// int		getDeviceChannels() const;
// double	getSamplingRate() const;
// long		getFrameCount() const;			-- number of frames rec'd or played


// This struct allows us to hide all Jack implementation details within
// this source file.

struct JackAudioDevice::Impl {
	// Put all class-specific state in here.  You can also add a constructor
	// to make sure all state is initialized, or do it by hand in the main
	// class constructor directly below.  Ditto with destructor.
	jack_client_t *client;
	int numinports;
	int numoutports;
	jack_port_t **inports;
	jack_port_t **outports;

	static int runProcess(jack_nframes_t nframes, void *context);
};

int
JackAudioDevice::Impl::runProcess(jack_nframes_t nframes, void *context)
{
	return 0;
}

JackAudioDevice::JackAudioDevice() : _impl(new Impl)
{
	_impl->client = NULL;
	_impl->numinports = 0;
	_impl->numoutports = 0;
	_impl->inports = NULL;
	_impl->outports = NULL;
}

JackAudioDevice::~JackAudioDevice()
{ 
	// Free any Impl state if not done in Impl::~Impl().
	close();
	delete _impl;
}

// This is by far the most complex method.  
// run() is called by ThreadedAudioDevice in a newly spawned thread.
// During runCallback(), the application will call sendFrames() or getFrames(),
// or if it is done, the callback returns false, which is our signal to
// finish up.

void
JackAudioDevice::run()
{
	// waitForDevice() waits on the descriptor you passed to setDevice() until
	// the device is ready to give/get audio.  It returns non-zero if 
	// AudioDevice::stop() is called, to allow the loop to exit.

	while (waitForDevice(0) == 0) {
		if (runCallback() != true) {
			break;
		}
	}
	// Do any HW-specific flushing, etc. here.

	// Now call the stop callback.
	stopCallback();
}

// doOpen() is called by AudioDevice::open() to do the class-specific opening
// of the audio port, HW, device, etc.  and set up any remaining Impl state.
//
// The assumption is that the open of the HW will return a integer file
// descriptor that we can wait on.  Before exiting this method, call
// setDevice() and hand it that descriptor.  It is used by waitForDevice().
//
// 'mode' has a direction portion and a bit to indicate if the device is being
// run in passive mode (does not spawn its own thread to handle I/O).
// You are guaranteed that doOpen() will NOT be called if you are already open.

int
JackAudioDevice::doOpen(int mode)
{
	// Initialize any Impl state if not done in Impl::Impl().

	_impl->client = jack_client_new("RTcmix");
	if (_impl->client == 0)
		return error("JACK server not running?");
	if (jack_set_process_callback(_impl->client, _impl->runProcess,
			(void *) this) != 0)
		return error("Could not register JACK process callback.");
	jack_on_shutdown(_impl->client, _impl->shutdown, (void *) this);

	switch (mode & DirectionMask) {
	case Playback:
		break;
	case Record:
		break;
	case RecordPlayback:
		break;
	default:
		return error("AudioDevice: Illegal open mode.");
	}
	return error("Not implemented");
}

// doClose() is called by AudioDevice::close() to do the class-specific closing
// of the audio port, HW, device, etc.  You are guaranteed that doClose() will
// NOT be called if you are already closed.

int
JackAudioDevice::doClose()
{
	int err = jack_client_close(_impl->client);
	if (err != 0)
		return error("Error closing JACK.");
	return error("Not implemented");
}

// doStart() is called by AudioDevice::start() to do class-specific calls which
// notify the HW to begin recording, playing, or both.

int
JackAudioDevice::doStart()
{
	return error("Not implemented");
}

// This does nothing under RTcmix, so can be left as-is.

int
JackAudioDevice::doPause(bool)
{
	return error("Not implemented");
}

// doSetFormat() is called by AudioDevice::setFormat() and by
// AudioDevice::open().  Here is where you configure your HW, setting it to the
// format which will best handle the format passed in.  Note that it is NOT
// necessary for the HW to match the input format except in sampling rate;  The
// base class can handle most format conversions.
//
// 'sampfmt' is the format of the data passed to AudioDevice::getFrames() or
//	AudioDevice::sendFrames(), and has three attributes:
//	1) The actual type of the format, retrieved via MUS_GET_FORMAT(sampfmt)
//	2) The interleave (true or false) retrieved via MUS_GET_INTERLEAVE(sampfmt)
//	3) Whether the samples (when float) are normalized, retrieved via
//		MUS_GET_NORMALIZED(sampfmt)
//
// At the end of this method, you must call setDeviceParams() to notify the
// base class what format *you* need the audio data to be in.

int
JackAudioDevice::doSetFormat(int sampfmt, int chans, double srate)
{
	// Insure that RTcmix sampling rate matches JACK rate.
	jack_nframes_t jsrate = jack_get_sample_rate(_impl->client);
	if (jsrate != (jack_nframes_t) srate) {
		char msg[256];
		snprintf(msg, 256, "RTcmix sampling rate (set in rtsetparams) must match "
					"JACK sampling rate (currently %d)", jsrate);
		return error(msg);
	}

	int deviceFormat = MUS_GET_FORMAT(sampfmt);

	return error("Not implemented");
}

// Connect JACK ports (must be done after activating)
int
JackAudioDevice::connectPorts()
{
}

// doSetQueueSize() is called by AudioDevice::setQueueSize() to allow
// HW-specific configuration of internal audio queue sizes.  The values handed
// in via address represent the size **in frames** of the buffers that will be
// handed to doGetFrames() and/or doSendFrames(), and the number of such
// buffers the application would like to have queued up for robustness.  The
// actual frame count as determined by your HW *must* be reported back to the
// caller via 'pWriteSize'.  If you cannot match *pCount, just do the best you
// can, but do not fail if you cannot match it.

int
JackAudioDevice::doSetQueueSize(int *pWriteSize, int *pCount)
{
	jack_nframes_t jackBufSize = jack_get_buffer_size(_impl->client);
// FIXME: I doubt this is right...
	jack_nframes _bufSize = *pWriteSize * *pCount;
	if (_bufSize != jackBufSize) {
		*pWriteSize = jackBufSize / pCount;
		// notify user
	}

	// doSetQueueSize is the last method called (indirectly) by
	// create_audio_devices, so it's our last chance to do the rest of the setup:
	// activating our JACK client and connecting JACK ports.  We do this here
	// because we must get the current JACK buffer size *before* activating.

	if (jack_activate(_impl->client) != 0)
		return error("Error activating JACK client.");

	int status = connectPorts();
	if (status != 0)
		return status;

	return 0;
}

// doGetFrames() is called by AudioDevice::getFrames() during record.
// The format of 'frameBuffer' will be the format **YOU** specified via
// setDeviceParams() above.  It will be converted into the 'frame format'
// by a base class.  Here is where you fill frameBuffer from your audio HW.

int
JackAudioDevice::doGetFrames(void *frameBuffer, int frameCount)
{
	return error("Not implemented");
}

// doSendFrames() is called by AudioDevice::sendFrames() during playback.
// The format of 'frameBuffer' will be the format **YOU** specified via
// setDeviceParams() above.  It was converted from the 'frame format'
// by a base class.   Here is where you hand the audio in frameBuffer to you
// HW.

int
JackAudioDevice::doSendFrames(void *frameBuffer, int frameCount)
{
	return error("Not implemented");
}

// Return true if the passed in device descriptor matches one that this device
// can understand.

bool JackAudioDevice::recognize(const char *desc)
{
	return false;
}

// If your audio device(s) needs a string descriptor, it will come in via
// 'inputDesc' and/or 'outputDesc', allowing you to specify different HW for
// record and play.

AudioDevice *JackAudioDevice::create(const char *inputDesc,
	const char *outputDesc, int mode)
{
	return new JackAudioDevice;
}

#endif	// JACK
