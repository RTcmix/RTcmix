// AudioFileDevice.cpp
//
// Allows applications to write audio to disk just as if they were writing
// to an audio device.  Performs sample fmt conversion between float and 
// other formats.  Constructor specifies audio file type, the file's sample 
// format, and options concerning format of data.

#include "AudioFileDevice.h"
#include <math.h>       /* for fabs */
#include <sndlibsupport.h>
#include <byte_routines.h>
#include <globals.h>	// MAXBUS, etc.
#include <assert.h>
#ifdef linux
#include <unistd.h>
#endif

struct AudioFileDevice::Impl {
	char						*path;
	int							frameSize;
	bool						stopping;
	bool						closing;
	bool						paused;
	int							inputFormat;	// What gets handed to device.
	int							channels;		// Cached.
	int							fileType;		// wave, aiff, etc.
	int							fileFormat;	// What gets written to file.
	double						sampRate;		// For both.
	void						*convertBuffer;
	bool						checkPeaks;
	bool						normalizeFloats;
	float						peaks[MAXBUS];
	long						peakLocs[MAXBUS];
};

AudioFileDevice::AudioFileDevice(const char *path,
								 int fileType,
								 int fileFmt,
								 int fileOptions)
	: _impl(new Impl)
{
	_impl->path = (char *)path;
	_impl->frameSize = 1024;
	_impl->stopping = false;
	_impl->closing = false;
	_impl->paused = false;
	_impl->inputFormat = MUS_UNSUPPORTED;
	_impl->channels = 0;
	_impl->fileType = fileType;
	_impl->fileFormat = fileFmt;
	_impl->convertBuffer = NULL;
	_impl->checkPeaks = fileOptions & CheckPeaks;
	_impl->normalizeFloats = fileOptions & NormalizeFloats;
	for (int n = 0; n < MAXBUS; ++n) {
		_impl->peaks[n] = 0.0;
		_impl->peakLocs[n] = 0;
	}
}

AudioFileDevice::~AudioFileDevice()
{
//	printf("AudioFileDevice::~AudioFileDevice\n");
	close();
	delete [] (char *) _impl->convertBuffer;
	delete _impl;
}

// We override open() in order to call setFormat() BEFORE doOpen().

int AudioFileDevice::open(int mode, int sampfmt, int chans, double srate)
{
//	printf("AudioFileDevice::open\n");

	if ((mode & Record) != 0)
		return error("Record from file device not supported");

	if (!IS_FLOAT_FORMAT(sampfmt)) {
		return error("Only floating point buffers accepted as input");
	}
	
	int status = 0;
	if (!isOpen()) {
		setState(Open);	// To allow setFormat.
		if ((status = setFormat(sampfmt, chans, srate)) == 0) {
			if ((status = doOpen(mode)) == 0) {
				setState(Configured);
			}
			else {
				setState(Closed);
			}
		}
	}
	return status;
}

double AudioFileDevice::getPeak(int chan, long *pLocation)
{
	*pLocation = _impl->peakLocs[chan];
	return _impl->peaks[chan];
}

int AudioFileDevice::doOpen(int mode)
{
	assert(!(mode & Record));
	setMode(mode);
	
	int fd = sndlib_create((char *)_impl->path, _impl->fileType,
						   _impl->fileFormat, (int)_impl->sampRate, 
						   _impl->channels);
	setDevice(fd);
	_impl->closing = false;
	resetFrameCount();
	return (fd > 0) ? 0 : -1;
}

int AudioFileDevice::doClose()
{
	int status = 0;
	if (!_impl->closing) {
		_impl->closing = true;
		if (_impl->checkPeaks) {
			(void) sndlib_put_header_comment(device(),
											 _impl->peaks,
											 _impl->peakLocs,
											 NULL);
		}
		// Last arg is samples not frames, for some archaic reason.
		status = sndlib_close(device(),
							  true,
							  _impl->fileType,
							  _impl->fileFormat,
							  getFrameCount() * _impl->channels);
		setDevice(-1);
	}
	return status;
}

int AudioFileDevice::doStart()
{
	_impl->stopping = false;
	_impl->paused = false;
//	printf("AudioFileDevice::doStart: starting thread\n");
	return ThreadedAudioDevice::startThread();
}

int AudioFileDevice::doPause(bool paused)
{
	_impl->paused = paused;
	return 0;
}

int AudioFileDevice::doSetFormat(int sampfmt, int chans, double srate)
{
	_impl->inputFormat = sampfmt;
	_impl->channels = chans;
	_impl->sampRate = srate;
	return 0;
}

int AudioFileDevice::doSetQueueSize(int *queueFrames)
{
	int queueBytes = mus_data_format_to_bytes_per_sample(_impl->fileFormat)
					 * _impl->channels
					 * *queueFrames;
	_impl->convertBuffer = (void *) new char[queueBytes];
	if (!_impl->convertBuffer)
		return error("AudioFileDevice::doSetQueueSize: memory allocation failure");
	return 0;
}

int AudioFileDevice::doGetFrameCount() const
{
	return frameCount();
}

int AudioFileDevice::doGetFrames(void *frameBuffer, int frameCount)
{
	return error("Reading from file device not yet supported");
}

int	AudioFileDevice::doSendFrames(void *frameBuffer, int frameCount)
{
	if (!_impl->convertBuffer)
		return error("Queue size was not set for audio file device.");
	// Convert sample format if necessary.
	void *outbuffer = convertSamples(frameBuffer, frameCount);
	int bytesToWrite = frameCount * _impl->channels * mus_data_format_to_bytes_per_sample(_impl->fileFormat);
	int bytesWritten = ::write(device(), outbuffer, bytesToWrite);
	if (bytesWritten < 0) {
		return error("Error writing to file.");
	}
	else if (bytesWritten < bytesToWrite) {
		return error("Incomplete write to file (disk full?)");
	}
	incrementFrameCount(frameCount);
	return frameCount;
}

void AudioFileDevice::run()
{
	Callback runCallback = getRunCallback();
//	printf("AudioFileDevice::run: TOF\n");
	assert(!isPassive());	// Cannot call this method when passive!
	
	while (_impl->stopping == false) {
		while (_impl->paused) {
#ifdef linux
			::usleep(1000);
#endif
		}
		if ((*runCallback)(this, getRunCallbackContext()) != true) {
//			printf("AudioFileDevice::run: callback returned false\n");
			break;
		}
	}
	// If we stopped due to callback being done, set the state so that the
	// call to close() does not attempt to call stop, which we cannot do in
	// this thread.  Then, check to see if we are being closed by the main
	// thread before calling close() here, to avoid a reentrant call.
	if (!_impl->stopping) {
		setState(Configured);
		if (!_impl->closing) {
//			printf("AudioFileDevice::run: calling close()\n");
			close();
		}
	}
	Callback stopCallback = getStopCallback();
	if (stopCallback) {
//		printf("AudioFileDevice::run: calling stop callback\n");
		(*stopCallback)(this, getStopCallbackContext());
	}
//	printf("AudioFileDevice::run: thread exiting\n");
}

void *AudioFileDevice::convertSamples(void *buffer, int frames)
{
	const int chans = _impl->channels;
	if (IS_FLOAT_FORMAT(_impl->fileFormat)) {
		// Will not have gone through limiter, so do peak check here.
		if (_impl->checkPeaks) {
			long bufStartSamp = frameCount();
			float *peaks = _impl->peaks;
			for (int c = 0; c < chans; ++c) {
				float *bp = &((float *) buffer)[c];
				for (int n = 0; n < frames; ++n, bp += chans) {
					double fabsamp = fabs((double) *bp);
					if (fabsamp > (double) peaks[c]) {
						peaks[c] = (float) fabsamp;
						_impl->peakLocs[c] = bufStartSamp + n;	// frame count
					}
				}
			}
		}
	}
	
	// Note: Otherwise, peak check was done in the limiter
	
	if (_impl->inputFormat == _impl->fileFormat) {
		return buffer;
	}
	
	static const float NORM_FLOAT_FACTOR = (1.0 / 32768.0);

	// Otherwise...
	const int arrayLen = _impl->channels * frames;
	float *fbuffer = (float *) buffer;
	unsigned char *bufp = (unsigned char *) _impl->convertBuffer;
	const int bytesPerSamp = mus_data_format_to_bytes_per_sample(_impl->fileFormat);
	int i;
	
	switch (_impl->fileFormat) {
	case MUS_BSHORT:    
		for (i = 0; i < arrayLen; ++i, bufp += 2)
			m_set_big_endian_short(bufp, (short) fbuffer[i]);
		break;

	case MUS_LSHORT:    
		for (i = 0; i < arrayLen; ++i, bufp += 2)
			m_set_little_endian_short(bufp, (short) fbuffer[i]);
		break;

	case MUS_BFLOAT:    
		if (_impl->normalizeFloats) {
			for (i = 0; i < arrayLen; ++i, bufp += 4)
    			m_set_big_endian_float(bufp,
                            		   fbuffer[i] * NORM_FLOAT_FACTOR);
		}
		else {
			for (i = 0; i < arrayLen; ++i, bufp += 4)
    			m_set_big_endian_float(bufp, fbuffer[i]);
		}
		break;

	case MUS_LFLOAT:    
		if (_impl->normalizeFloats) {
			for (i = 0; i < arrayLen; ++i, bufp += 4)
				m_set_little_endian_float(bufp,
										  fbuffer[i] * NORM_FLOAT_FACTOR);
		}
		else {
			for (i = 0; i < arrayLen; ++i, bufp += 4)
				m_set_little_endian_float(bufp, fbuffer[i]);
		}
		break;

	case MUS_B24INT:
		for (i = 0; i < arrayLen; ++i, bufp += 3) {
			int samp = (int) ((fbuffer[i] * NORM_FLOAT_FACTOR) * (1 << 23));
			bufp[0] = (samp >> 16);
			bufp[1] = (samp >> 8);
			bufp[2] = (samp & 0xFF);
		}
		break;

	case MUS_L24INT:
		for (i = 0; i < arrayLen; ++i, bufp += 3) {
			int samp = (int) ((fbuffer[i] * NORM_FLOAT_FACTOR) * (1 << 23));
			bufp[2] = (samp >> 16);
			bufp[1] = (samp >> 8);
			bufp[0] = (samp & 0xFF);
		}
		break;

	default:
		assert(!"rtwritesamps: unknown output data format!");
		break;/*NOTREACHED*/
	}
	return _impl->convertBuffer;
}

// template <class In, class Out> float getScale(In *, Out *);
// 
// static const float oneOverShortMax = 0.0000305177578f;
// 
// template <>
// inline float getScale(float *, short *) { return 1.0f; }
// 
// template <>
// inline float getScale(short *, float *) { return 1.0f; }
// 
// template <class In, class Out>
// void convertArray(void *input, void *output, const int len) {
// 	In *inptr = (In *) input;
// 	Out *outptr = (Out *) output;
// 	const float scale = getScale(inptr, outptr);
// 	for (int n = 0; n < len; ++n) {
// 		outptr[n] = (Out) (inptr[n] * scale);
// 	}
// }

// void *AudioFileDevice::convertSamples(void *buffer, int frames)
// {
// 	if (_impl->inputFormat == _impl->fileFormat) {
// 		return buffer;
// 	}
// 	// Otherwise...
// 	const int arrayLen = _impl->channels * frames;
// 	switch (_impl->inputFormat) {
// 	case MUS_LSHORT:
// 		convertArray<short, float>(buffer, _impl->convertBuffer, arrayLen);
// 		break;
// 	case MUS_LFLOAT:
// 		convertArray<float, short>(buffer, _impl->convertBuffer, arrayLen);
// 		break;
// 	default:
// 		assert(0);
// 	}
// 	return _impl->convertBuffer;
// }
