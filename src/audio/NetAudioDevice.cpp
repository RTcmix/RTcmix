// NetAudioDevice.cpp

#if defined (NETAUDIO)

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#include "NetAudioDevice.h"
#include "sndlibsupport.h"

// This subclass allows a network audio connection between two machines.  The
// input NetAudioDevice will block in start(), waiting for a connection.  When
// a connection is established, a one-way "handshake" verifies the format of
// the audio coming over the socket, and the input device (re)configures itself
// to handle the audio format, if possible.  If the input connection is lost,
// the input NetAudioDevice returns to the point where it blocks, and the cycle
// may repeat.
//
// The output NetAudioDevice can only be open()d if there is an input device
// waiting for it.  Once a connection is established, the output device sends
// its "handshake" data to notify the listener of the audio format.  Currently
// the NetAudioDevices only communicate using short integer audio.

static const int kDefaultSockNumber = 9999;

// The cookie is used to identify whether the data coming over the socket
// is (1) a valid stream and (2) little- or big-endian.

static const int kAudioFmtCookie = 0x12345678;
static const int kAudioFmtCookieSwapped = 0x78563412;

// This is the struct sent as the one-way handshake.

struct NetAudioFormat {
	int		cookie;			// kAudioFmtCookie
	int		fmt;			// always SHORT, for now.
	int		chans;
	float	sr;
	int		blockFrames;	// Number of frames per write
};

static const int kNetAudioFormat_Size = sizeof(NetAudioFormat);
static const int kNetAudioSampfmt = NATIVE_SHORT_FMT | MUS_INTERLEAVED;

struct NetAudioDevice::Impl {
	Impl() : hostname(NULL), sockno(-1), sockdesc(-1), swapped(false) {}
	char				*hostname;
	int					sockno;
	struct sockaddr_in	sss;
	int					sockdesc;			// as opened by socket()
	int					framesPerWrite;		// set via doSetQueueSize()
	bool				swapped;			// true if we need to swap
};

inline bool NetAudioDevice::connected() { return device() > 0; }

NetAudioDevice::NetAudioDevice(const char *path) : _impl(new Impl)
{
	char *substr = strstr(path, ":");
	if (substr != NULL) {	
		// Break path of form "hostname:sockno" into its components and store.
		int sepIndex = strlen(path) - strlen(substr);
		_impl->hostname = new char[sepIndex + 1];
		strncpy(_impl->hostname, path, sepIndex);
		_impl->hostname[sepIndex] = '\0';
		_impl->sockno = atoi(&path[sepIndex + 1]);
	}
	else {
		// Path was just "hostname"
		_impl->hostname = new char[strlen(path) + 1];
		strcpy(_impl->hostname, path);
		_impl->sockno = kDefaultSockNumber;
	}
}

NetAudioDevice::~NetAudioDevice()
{
	delete [] _impl->hostname;
	delete _impl;
}

bool NetAudioDevice::waitForDevice(unsigned int wTime)
{
	while (!connected()) {
		if (waitForConnect() == 0) {
			if (configure() < 0) {
				disconnect();
				continue;	// try again
			}
		}
		else
			return false;
	}
	return ThreadedAudioDevice::waitForDevice(wTime);
}

// This is by far the most complex method.  
// run() is called by ThreadedAudioDevice in a newly spawned thread.
// During runCallback(), the application will call sendFrames() or getFrames(),
// or if it is done, the callback returns false, which is our signal to
// finish up.

void
NetAudioDevice::run()
{
	// waitForDevice() waits on the descriptor you passed to setDevice() until
	// the device is ready to give/get audio.  It returns false if 
	// AudioDevice::stop() is called, to allow the loop to exit.
	unsigned waitMillis = 10000;
    while (waitForDevice(waitMillis) == true) {
        if (runCallback() != true) {
            break;
        }
 	}
	// If we stopped due to callback being done, set the state so that the
	// call to close() does not attempt to call stop, which we cannot do in
	// this thread.  Then, check to see if we are being closed by the main
	// thread before calling close() here, to avoid a reentrant call.
	
	if (!stopping()) {
		setState(Configured);
		if (!closing()) {
			close();
		}
	}
	
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
NetAudioDevice::doOpen(int mode)
{
	struct hostent *hp;
	int len = sizeof(_impl->sss);

	bzero(&_impl->sss, len);
	
	// Create the socket
	if( (_impl->sockdesc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return error("NetAudioDevice: socket call failed: ",
					 strerror(errno));
	}
	// set up the socket address
	_impl->sss.sin_family = AF_INET;
	_impl->sss.sin_port = htons(_impl->sockno);

	switch (mode & DirectionMask) {
	case Playback:
		if ((hp = gethostbyname(_impl->hostname)) == NULL) 
			return error("NetAudioDevice: gethostbyname failed: ",
						 strerror(errno));
		bcopy(hp->h_addr, &(_impl->sss.sin_addr.s_addr), hp->h_length);
		if (connect(_impl->sockdesc, (struct sockaddr *)&_impl->sss, len) < 0) {
			return error("NetAudioDevice: connect failed: ",
						 strerror(errno));
		}
		setDevice(_impl->sockdesc);
		break;
	case Record:
		_impl->sss.sin_addr.s_addr = INADDR_ANY;
		if (bind(_impl->sockdesc, (struct sockaddr *)&_impl->sss, len) < 0) {
			return error("NetAudioDevice: bind failed: ", 
						 strerror(errno));
		}
		break;
	default:
		return error("NetAudioDevice: Illegal open mode.");
	}
	return 0;
}

// doClose() is called by AudioDevice::close() to do the class-specific closing
// of the audio port, HW, device, etc.
// You are guaranteed that doClose() will NOT be called if you are already closed.

int
NetAudioDevice::doClose()
{
	int status;
	// In record, the data device descriptor is not
	// the socket descriptor.
	if (isRecording() && _impl->sockdesc > 0) {
		if (::close(_impl->sockdesc) == 0)
			_impl->sockdesc = -1;
	}
	return disconnect();
}

// doStart() is called by AudioDevice::start() to do class-specific calls which
// notify the HW to begin recording, playing, or both.

int
NetAudioDevice::doStart()
{
	int status = 0;
	if (isPlaying()) {
		NetAudioFormat netformat;
		netformat.cookie = kAudioFmtCookie;
		netformat.fmt = kNetAudioSampfmt;
		netformat.chans = getDeviceChannels();
		netformat.sr = (float) getSamplingRate();
		netformat.blockFrames = _impl->framesPerWrite;
		printf("NetAudioDevice::doStart: writing header to stream...\n");
		int wr;
		if ((wr = ::write(device(), &netformat, kNetAudioFormat_Size)) != kNetAudioFormat_Size)
		{
			return error("NetAudioDevice: unable to write header: ",
				  		 (wr >= 0) ? "partial or zero write" : strerror(errno));
		}
		printf("NetAudioDevice::doStart: wrote header\n");
	}
	else if (isRecording()) {
		bool ready = false;
		while (!ready) {
			if (waitForConnect() == 0) {
				if (configure() == 0)
					ready = true;	// connection OK
				else
					disconnect();	// connection was not compatible; retry.
			}
			else
				return -1;
		}
	}
	return ThreadedAudioDevice::startThread();
}

// This does nothing under RTcmix, so can be left as-is.

int
NetAudioDevice::doPause(bool)
{
	return error("Not implemented");
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

int
NetAudioDevice::doSetFormat(int sampfmt, int chans, double srate)
{
	// Always interleaved shorts.
	setDeviceParams(kNetAudioSampfmt, chans, srate);
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

int
NetAudioDevice::doSetQueueSize(int *pWriteSize, int *pCount)
{
	_impl->framesPerWrite = *pWriteSize;
	return 0;
}

// doGetFrames() is called by AudioDevice::getFrames() during record.
// The format of 'frameBuffer' will be the format **YOU** specified via
// setDeviceParams() above.  It will be converted into the 'frame format'
// by a base class.  Here is where you fill frameBuffer from your audio HW.

int
NetAudioDevice::doGetFrames(void *frameBuffer, int frameCount)
{
	int bytesToRead = frameCount * getDeviceBytesPerFrame();
//	printf("NetAudioDevice::doGetFrames: reading %d bytes from stream...\n", bytesToRead);
	int bytesRead = ::read(device(), frameBuffer, bytesToRead);
//	printf("NetAudioDevice::doGetFrames: read %d bytes\n", bytesRead);
	if (bytesRead == 0) {
		printf("NetAudioDevice::doGetFrames: connection broke -- disconnecting\n");
		memset(frameBuffer, 0, bytesToRead);
		disconnect();
		return 0;
	}
	if (bytesRead < 0) {
		return error("NetAudioDevice: read failed: ", strerror(errno));
	}
	// For now, we swap here.  This will be handled by base class.
	if (_impl->swapped) {
		short *sFrame = (short *) frameBuffer;
		const int count = frameCount * getDeviceChannels();
		for (int n = 0; n < count; ++n) {
			const unsigned short tmp = (unsigned short) sFrame[n];
			sFrame[n] = short((tmp >> 8) | (tmp << 8) & 0xffff);
		}
	}
	int framesRead = bytesRead / getDeviceBytesPerFrame();
	incrementFrameCount(framesRead);
	return framesRead;
}

// doSendFrames() is called by AudioDevice::sendFrames() during playback.
// The format of 'frameBuffer' will be the format **YOU** specified via
// setDeviceParams() above.  It was converted from the 'frame format'
// by a base class.   Here is where you hand the audio in frameBuffer to you
// HW.

int
NetAudioDevice::doSendFrames(void *frameBuffer, int frameCount)
{
	int bytesToWrite = frameCount * getDeviceBytesPerFrame();
//	printf("NetAudioDevice::doSendFrames: writing %d bytes to stream...\n", bytesToWrite);
	int bytesWritten = ::write(device(), frameBuffer, bytesToWrite);
//	printf("NetAudioDevice::doSendFrames: wrote %d bytes\n", bytesWritten);
	if (bytesWritten <= 0) {
		return error("NetAudioDevice: write failed: ",
					 (bytesWritten < 0) ? strerror(errno) : "wrote zero bytes");
	}
	int framesWritten = bytesWritten / getDeviceBytesPerFrame();
	incrementFrameCount(framesWritten);
	return framesWritten;
}

int NetAudioDevice::waitForConnect()
{
	if (listen(_impl->sockdesc, 1) < 0) {
		return error("NetAudioDevice: listen failed: ", 
					 strerror(errno));
	}
	printf("NetAudioDevice: waiting for input connection...\n");
#ifdef MACOSX
	int len = sizeof(_impl->sss);
#else
	socklen_t len = sizeof(_impl->sss);
#endif
	int sockdev = accept(_impl->sockdesc, (struct sockaddr *)&_impl->sss, &len);
	if (sockdev < 0) {
		::close(_impl->sockdesc);
		return error("NetAudioDevice: accept failed: ", 
					 strerror(errno));
	}
	setDevice(sockdev);
	printf("NetAudioDevice: got connection\n");
	return 0;
}

int NetAudioDevice::disconnect()
{
	int status = 0;
	if ((status = ::close(device())) == 0)
		setDevice(-1);
	else
		error("NetAudioDevice: close failed: ", strerror(errno));
	return status;
}

inline unsigned
swap(unsigned ul) {
    return (ul >> 24) | ((ul >> 8) & 0xff00) | ((ul << 8) & 0xff0000) | (ul << 24);
}

inline int swap(int x) { return swap((unsigned)x); }

inline float
swap(float uf) {
    union { unsigned l; float f; } u;
    u.f = uf;
    u.l = swap(u.l);
    return (u.f);
}

int NetAudioDevice::configure()
{
	NetAudioFormat netformat;
	int rd;
//	printf("NetAudioDevice::configure(): reading header from stream...\n");
	if ((rd = ::read(device(), &netformat, kNetAudioFormat_Size)) != kNetAudioFormat_Size)
	{
		fprintf(stderr, "NetAudioDevice: unable to read header: %s",
				(rd >= 0) ? "partial or zero read" : strerror(errno));
		return error("NetAudioDevice: unable to read header: ",
					 (rd >= 0) ? "partial or zero read" : strerror(errno));
	}
//	printf("NetAudioDevice: read header\n");
	switch (netformat.cookie) {
	case kAudioFmtCookie:
		_impl->swapped = false;
		break;
	case kAudioFmtCookieSwapped:
		_impl->swapped = true;
		break;
	default:
		fprintf(stderr, "NetAudioDevice: missing or corrupt header: cookie = 0x%x", netformat.cookie);
		return error("NetAudioDevice: missing or corrupt header");
	}
	if (_impl->swapped) {
		printf("debug: header cookie was opposite endian\n");
		netformat.fmt = swap(netformat.fmt);
		netformat.chans = swap(netformat.chans);
		netformat.sr = swap(netformat.sr);
		netformat.blockFrames = swap(netformat.blockFrames);
	}
	// Now make sure we can handle the format
	if (!IS_SHORT_FORMAT(netformat.fmt)) {
		fprintf(stderr, "NetAudioDevice: unknown or unsupported audio format\n");
		return error("NetAudioDevice: unsupported audio format");
	}
	// For now we cannot handle buffer size or channel mismatches.
	if (netformat.blockFrames != _impl->framesPerWrite) {
		fprintf(stderr, "NetAudioDevice: sender and receiver RTBUFSAMPS must match\n");
		return error("NetAudioDevice: audio buffer size mismatch");
	}
	else if (netformat.chans != getDeviceChannels()) {
		fprintf(stderr, "NetAudioDevice: sender and receiver channel counts must match\n");
		return error("NetAudioDevice: audio channel count mismatch");
	}
	// Just give a warning for SR mismatch.
	if (netformat.sr != getSamplingRate()) {
		fprintf(stderr, "NetAudioDevice: warning: SR mismatch.  Playing at incorrect rate.\n");
	}
	setDeviceParams(netformat.fmt, netformat.chans, netformat.sr);
//	printf("NetAudioDevice::configure(): resetting conversion routines\n");
	return resetFormatConversion();
}

bool NetAudioDevice::recognize(const char *desc)
{
	return (desc != NULL) && strncmp(desc, "net:", 4) == 0;
}

AudioDevice *NetAudioDevice::create(const char *inputDesc, const char *outputDesc, int mode)
{
	AudioDevice *theDevice = NULL;
	// We dont support full duplex for this class.
	if ((mode & AudioDevice::DirectionMask) != RecordPlayback) {
		const char *desc = inputDesc ? inputDesc : outputDesc;
		// Strip off the "net:" from the beginning of the descriptor.
		theDevice  = new NetAudioDevice(&desc[4]);
	}
	return theDevice;
}

#endif	// NETAUDIO
