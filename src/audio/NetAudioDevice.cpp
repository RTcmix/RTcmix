// NetAudioDevice.cpp

#if defined (NETAUDIO)

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
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

#define DEBUG 0

#if DEBUG > 0
#define PRINT0 if (1) printf
#define PRINT1 if (0) printf
#else
#if DEBUG > 1
#define PRINT0 if (1) printf
#define PRINT1 if (1) printf
#else
#define PRINT0 if (0) printf
#define PRINT1 if (0) printf
#endif
#endif

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
	Impl() : hostname(NULL), sockno(-1), sockdesc(-1) {}
	char				*hostname;
	int					sockno;
	struct sockaddr_in	sss;
	int					sockdesc;			// as opened by socket()
	int					framesPerWrite;		// set via doSetQueueSize()
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

void
NetAudioDevice::run()
{
	PRINT1("NetAudioDevice::run: top of loop\n");
	// waitForDevice() waits on the descriptor you passed to setDevice() until
	// the device is ready to give/get audio.  It returns false if 
	// AudioDevice::stop() is called, to allow the loop to exit.
	unsigned waitMillis = 10000;
    while (waitForDevice(waitMillis) == true) {
        if (runCallback() != true) {
            break;
        }
 	}
	PRINT1("NetAudioDevice::run: after loop\n");
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
	PRINT1("NetAudioDevice::run: calling stop callback\n");	
	// Now call the stop callback.
	stopCallback();
}


int
NetAudioDevice::doOpen(int mode)
{
	struct hostent *hp;
	int len = sizeof(_impl->sss);

	PRINT0("NetAudioDevice::doOpen()\n");

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

int
NetAudioDevice::doClose()
{
	int status;
	// In record, the data device descriptor is not
	// the socket descriptor.
	PRINT0("NetAudioDevice::doClose()\n");
	closing(true);	// This allows waitForConnect() to exit.
	if (isRecording() && _impl->sockdesc > 0) {
		if (::close(_impl->sockdesc) == 0)
			_impl->sockdesc = -1;
	}
	return disconnect();
}

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
		PRINT1("NetAudioDevice::doStart: writing header to stream...\n");
		int wr;
		if ((wr = ::write(device(), &netformat, kNetAudioFormat_Size)) != kNetAudioFormat_Size)
		{
			return error("NetAudioDevice: unable to write header: ",
				  		 (wr >= 0) ? "partial or zero write" : strerror(errno));
		}
		PRINT1("NetAudioDevice::doStart: wrote header\n");
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

int
NetAudioDevice::doSetFormat(int sampfmt, int chans, double srate)
{
	// Always interleaved shorts.
	setDeviceParams(kNetAudioSampfmt, chans, srate);
	return 0;
}

int
NetAudioDevice::doSetQueueSize(int *pWriteSize, int *pCount)
{
	_impl->framesPerWrite = *pWriteSize;
	return 0;
}

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
	int framesRead = bytesRead / getDeviceBytesPerFrame();
	incrementFrameCount(framesRead);
	return framesRead;
}

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
	// Make sure we wont block.
	fcntl(_impl->sockdesc, F_SETFL, O_NONBLOCK);
	// Select on socket, waiting until there is a connection to accept.
	// This allows us to break out if device stops or closes.
	fd_set rfdset;
	struct timeval timeout;
	const int nfds = _impl->sockdesc + 1;
	int ret;
	FD_ZERO(&rfdset);
	do {
		if (closing() || stopping()) {
			PRINT0("breaking out of wait for stop or close\n");
			return -1;
		}
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;
		FD_SET(_impl->sockdesc, &rfdset);
//		printf("in select loop...\n");
	} while ((ret = select(nfds, &rfdset, NULL, NULL, &timeout)) == 0);
	if (ret == -1) {
		PRINT0("select() returned -1, breaking out of wait\n");
		return -1;
	}
#ifdef MACOSX
	int len = sizeof(_impl->sss);
#else
	socklen_t len = sizeof(_impl->sss);
#endif
	int sockdev = accept(_impl->sockdesc, (struct sockaddr *)&_impl->sss, &len);
	if (sockdev < 0) {
		::close(_impl->sockdesc);
		return error("NetAudioDevice: accept failed: ", strerror(errno));
	}
	setDevice(sockdev);
	printf("NetAudioDevice: got connection\n");
	return 0;
}

int NetAudioDevice::disconnect()
{
	int status = 0;
	const int dev = device();
	if (dev > 0) {
		// NOTE:  Still possible to have race condition with _device.
		if ((status = ::close(dev)) == 0)
			setDevice(-1);
		else
			error("NetAudioDevice: close failed: ", strerror(errno));
	}
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
	bool swapped = false;
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
		swapped = false;
		break;
	case kAudioFmtCookieSwapped:
		swapped = true;
		break;
	default:
		fprintf(stderr, "NetAudioDevice: missing or corrupt header: cookie = 0x%x", netformat.cookie);
		return error("NetAudioDevice: missing or corrupt header");
	}
	if (swapped) {
		PRINT0("debug: header cookie was opposite endian\n");
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
	netformat.fmt |= MUS_INTERLEAVED;	// Just for good measure!

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
	PRINT1("NetAudioDevice::configure(): resetting conversion routines\n");
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
