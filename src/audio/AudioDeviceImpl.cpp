// AudioDeviceImpl.cpp

#include "AudioDeviceImpl.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <sndlibsupport.h>	// RTcmix header

AudioDeviceImpl::AudioDeviceImpl() 
	: _mode(Unset), _state(Closed),
	  _frameFormat(MUS_UNSUPPORTED), _deviceFormat(MUS_UNSUPPORTED),
	  _frameChannels(0), _deviceChannels(0), _samplingRate(0.0),
	  _runCallback(NULL), _stopCallback(NULL),
	  _runCallbackContext(NULL), _stopCallbackContext(NULL),
	  _convertBuffer(NULL),
	  _recConvertFunction(NULL), _playConvertFunction(NULL)
{
}

AudioDeviceImpl::~AudioDeviceImpl()
{
	/* if this asserts, close() is not being called before destructor */
	assert(_convertBuffer == NULL);
}

int	AudioDeviceImpl::setFrameFormat(int frameSampFmt, int frameChans)
{
	_frameFormat = frameSampFmt;
	_frameChannels = frameChans;
	return 0;
}

int AudioDeviceImpl::open(int mode, int sampfmt, int chans, double srate)
{
//	printf("AudioDeviceImpl::open\n");
	_lastErr[0] = 0;
	if (!IS_FLOAT_FORMAT(MUS_GET_FORMAT(sampfmt))) {
		return error("Only floating point buffers are accepted as input");
	}
	setMode(mode);
	int status = 0;
	close();
	if ((status = doOpen(mode)) == 0) {
		setState(Open);
		if ((status = setFormat(sampfmt, chans, srate)) != 0)
			close();
	}
	return status;
}

int AudioDeviceImpl::close()
{
//	printf("AudioDeviceImpl::close -- begin\n");
	int status = 0;
	if (isOpen()) {
//		printf("AudioDeviceImpl::close: was open, calling stop()\n");
		stop();
//		printf("AudioDeviceImpl::close: now calling doClose()\n");
		if ((status = doClose()) == 0) {
			destroyConvertBuffer();
			setState(Closed);
			_frameFormat = MUS_UNSUPPORTED;
			_frameChannels = 0;
			_samplingRate = 0;
//			printf("AudioDeviceImpl::close: state now set to Closed\n");
		}
	}
//	printf("AudioDeviceImpl::close -- finish\n");
	return status;
}

int AudioDeviceImpl::start(AudioDevice::Callback callback, void *context)
{
	int status = 0;
	if (!isRunning()) {
		_runCallback = callback;
		_runCallbackContext = context;
		State oldState = getState();
		setState(Running);
		if ((status = doStart()) != 0) {
			setState(oldState);
		}
	}
	return status;
}

int AudioDeviceImpl::pause(bool willPause)
{
	int status = 0;
	if (isRunning() && isPaused() != willPause) {
		if ((status = doPause(willPause)) == 0) {
			setState(willPause ? Paused : Running);
		}
	}
	return status;
}

int AudioDeviceImpl::setStopCallback(Callback stopCallback, void *callbackContext)
{
	_stopCallback = stopCallback;
	_stopCallbackContext = callbackContext;
	return 0;
}

int AudioDeviceImpl::stop()
{
//	printf("AudioDeviceImpl::stop -- begin\n");
	int status = 0;
	if (isRunning()) {
//		printf("AudioDeviceImpl::stop: was running, calling doStop()\n");
		if ((status = doStop()) == 0) {
			setState(Configured);
		}
	}
	_runCallback = NULL;
	_runCallbackContext = NULL;
//	printf("AudioDeviceImpl::stop -- finish\n");
	return status;
}

int AudioDeviceImpl::getFrames(void *frameBuffer, int frameCount)
{
	int status = 0;
	if (isRecording()) {
		// XXX this is a hack to avoid an extra buffer copy.
		if (getFrameFormat() == getDeviceFormat() &&
			isFrameInterleaved() == isDeviceInterleaved()) {
			doGetFrames(frameBuffer, frameCount);
		}
		else {
			doGetFrames(_convertBuffer, frameCount);
			convertFrame(_convertBuffer, frameBuffer, frameCount, true);
		}
	}
	else
		status = error("Not in record mode");
	return status;
}

int AudioDeviceImpl::sendFrames(void *frameBuffer, int frameCount)
{
	int status = 0;
	if (isPlaying()) {
		void *sendBuffer = convertFrame(frameBuffer,
										_convertBuffer, 
										frameCount, 
										false);
		status = doSendFrames(sendBuffer, frameCount);
	}
	else
		status = error("Not in playback mode");
	return status;
}

const char *AudioDeviceImpl::getLastError() const {
	return _lastErr;
}

int AudioDeviceImpl::error(const char *msg, const char *msg2)
{
	sprintf(_lastErr, "AudioDevice: %s%s", msg, msg2 ? msg2 : "");
	return -1;
}

int AudioDeviceImpl::setFormat(int sampfmt, int chans, double srate)
{
	if (isOpen()) {
		if (doSetFormat(sampfmt, chans, srate) == 0) {
			assert(_deviceFormat != 0);
			assert(_deviceChannels != 0);
			assert(_samplingRate != 0.0);
			setState(Configured);
			return 0;
		}
		else return -1;
	}
	return error("Audio device not open");
}

// Code for creating and destroying interleaved conversion buffer

void *
AudioDeviceImpl::createInterleavedBuffer(int fmt, int chans, int len)
{
	void *buffer = NULL;
	if (IS_FLOAT_FORMAT(fmt)) {
		buffer = (void *) new float[chans * len];
	}
	else if (IS_SHORT_FORMAT(fmt)) {
		buffer = (void *) new short[chans * len];
	}
	else error("createInterleavedBuffer: unknown sample format!");
	return buffer;
}

void 
AudioDeviceImpl::destroyInterleavedBuffer(int fmt)
{
	if (IS_FLOAT_FORMAT(fmt)) {
		delete [] (float *) _convertBuffer;
	}
	else if (IS_SHORT_FORMAT(fmt)) {
		delete [] (short *) _convertBuffer;
	}
}

// Code for creating and destroying non-interleaved conversion buffer

template <class Type> Type **newNoninterleavedBuffer(int chans, int len) {
	Type **tbuf = new Type *[chans];
	if (tbuf) {
		for (int c = 0; c < chans; ++c) {
			tbuf[c] = new Type[len];
			if (tbuf[c] == NULL) {		// Unwind in event of memory failure
				for (int n = c - 1; n > 0; --n)
					delete [] tbuf[n];
				delete [] tbuf;
				return NULL;
			}
		}
	}
	return tbuf;
}

template <class Type> void deleteNoninterleavedBuffer(void *buf, int chans) {
	Type **tbuf = (Type **) buf;
	if (tbuf) {
		for (int c = 0; c < chans; ++c)
			delete [] tbuf[c];
		delete [] tbuf;
	}
}

void *
AudioDeviceImpl::createNoninterleavedBuffer(int fmt, int chans, int len)
{
	void *buffer = NULL;
	if (IS_FLOAT_FORMAT(fmt)) {
		buffer = (void *) newNoninterleavedBuffer<float>(chans, len);
	}
	else if (IS_SHORT_FORMAT(fmt)) {
		buffer = (void *) newNoninterleavedBuffer<short>(chans, len);
	}
	else error("createNoninterleavedBuffer: unknown sample format!");
	return buffer;
}

void
AudioDeviceImpl::destroyNoninterleavedBuffer(int fmt, int chans)
{
	if (IS_FLOAT_FORMAT(fmt)) {
		deleteNoninterleavedBuffer<float>(_convertBuffer, chans);
	}
	else if (IS_SHORT_FORMAT(fmt)) {
		deleteNoninterleavedBuffer<short>(_convertBuffer, chans);
	}
}

// createConvertBuffer(): Creates an interleaved or non-interleaved intermediate
// buffer if the input and device formats/interleaves do not match.
// Note that for both record and playback, the conversion buffer is the same
// format -- that of the HW.

int AudioDeviceImpl::createConvertBuffer(int frames)
{
	bool needFormatConversion = getFrameFormat() != getDeviceFormat();
	bool needInterleaveConversion = isFrameInterleaved() != isDeviceInterleaved();
	if (needFormatConversion || needInterleaveConversion) {
		if (isDeviceInterleaved())
			_convertBuffer = createInterleavedBuffer(getDeviceFormat(),
													 getDeviceChannels(),
													 frames);
		else
			_convertBuffer = createNoninterleavedBuffer(getDeviceFormat(),
														getDeviceChannels(),
														frames);
		if (!_convertBuffer)
			return error("AudioDeviceImpl::createConvertBuffer: memory allocation failure");
	}
	return 0;
}

void AudioDeviceImpl::destroyConvertBuffer()
{
	if (isDeviceInterleaved())
		destroyInterleavedBuffer(getDeviceFormat());
	else
		destroyNoninterleavedBuffer(getDeviceFormat(), getDeviceChannels());
	_convertBuffer = NULL;
}

int AudioDeviceImpl::setQueueSize(int *pWriteSize, int *pCount)
{
	int reqWriteSize = *pWriteSize;
	int reqCount = *pCount;
	switch (getState()) {
	case Configured:
		break;
	case Running:
	case Paused:
		if (isPassive())
			break;	// In passive mode we can do this anytime.
		// else
		*pWriteSize = -1;
		return error("Cannot set queue size while running");
	case Open:
		*pWriteSize = -1;
		return error("Cannot set queue size before setting format");
	case Closed:
		*pWriteSize = -1;
		return error("Audio device not open");
	default:
		*pWriteSize = -1;
		return error("Unknown state!");
	}
	
	if (doSetQueueSize(&reqWriteSize, &reqCount) == 0) {
		*pWriteSize = reqWriteSize;
		*pCount = reqCount;
		int status = createConvertBuffer(reqWriteSize * reqCount);
		if (status == 0) {
			status = setConvertFunctions(getFrameFormat(),
										 getDeviceFormat(),
										 isFrameInterleaved(),
										 isDeviceInterleaved());
		}
		return status;
	}
	*pWriteSize = -1;	// error condition
	return -1;
}

int AudioDeviceImpl::getDeviceBytesPerFrame() const
{
	return mus_data_format_to_bytes_per_sample(getDeviceFormat()) 
				* getDeviceChannels();
}

int AudioDeviceImpl::getFrameFormat() const
{
	return MUS_GET_FORMAT(_frameFormat);
}

int AudioDeviceImpl::getDeviceFormat() const
{
	return MUS_GET_FORMAT(_deviceFormat);
}

bool AudioDeviceImpl::isFrameInterleaved() const
{
	return IS_INTERLEAVED_FORMAT(_frameFormat);
}

bool AudioDeviceImpl::isDeviceInterleaved() const
{
	return IS_INTERLEAVED_FORMAT(_deviceFormat);
}

int AudioDeviceImpl::getFrameChannels() const
{
	return isOpen() ? _frameChannels : 0;
}

int AudioDeviceImpl::getDeviceChannels() const
{
	return  _deviceChannels;
}

double AudioDeviceImpl::getSamplingRate() const
{
	return _samplingRate;
}

long AudioDeviceImpl::getFrameCount() const
{
	return isOpen() ? doGetFrameCount() : 0;
}

void *
AudioDeviceImpl::convertFrame(void *inbuffer, void *outbuffer,
							  int frames, bool recording)
{
	const int chans = getDeviceChannels();
		
	if (_frameFormat == _deviceFormat) {
		return inbuffer;
	}
	else {
		assert(inbuffer != NULL);
		assert(outbuffer != NULL);
		if (recording) {
			assert(_recConvertFunction != NULL);
			(*_recConvertFunction)(inbuffer, outbuffer, chans, frames);
		}
		else {
			assert(_playConvertFunction != NULL);
			(*_playConvertFunction)(inbuffer, outbuffer, chans, frames);
		}
		return outbuffer;
	}
	
//	static const float NORM_FLOAT_FACTOR = (1.0 / 32768.0);

	// Otherwise...
//	const int arrayLen = chans * frames;
//	float *fbuffer = (float *) buffer;
//	unsigned char *bufp = (unsigned char *) _convertBuffer;
//	int i;
	
//	switch (devFormat) {
//
//	case MUS_BFLOAT:    
//		if (_impl->normalizeFloats) {
//			for (i = 0; i < arrayLen; ++i, bufp += 4)
//    			m_set_big_endian_float(bufp,
//                           		   fbuffer[i] * NORM_FLOAT_FACTOR);
//		}
//		else {
//			for (i = 0; i < arrayLen; ++i, bufp += 4)
//    			m_set_big_endian_float(bufp, fbuffer[i]);
//		}
//		break;
//
//	case MUS_LFLOAT:    
//		if (_impl->normalizeFloats) {
//			for (i = 0; i < arrayLen; ++i, bufp += 4)
//				m_set_little_endian_float(bufp,
//										  fbuffer[i] * NORM_FLOAT_FACTOR);
//		}
//		else {
//			for (i = 0; i < arrayLen; ++i, bufp += 4)
//				m_set_little_endian_float(bufp, fbuffer[i]);
//		}
//		break;

// 	case MUS_B24INT:
// 		for (i = 0; i < arrayLen; ++i, bufp += 3) {
// 			int samp = (int) ((fbuffer[i] * NORM_FLOAT_FACTOR) * (1 << 23));
// 			bufp[0] = (samp >> 16);
// 			bufp[1] = (samp >> 8);
// 			bufp[2] = (samp & 0xFF);
// 		}
// 		break;
// 
// 	case MUS_L24INT:
// 		for (i = 0; i < arrayLen; ++i, bufp += 3) {
// 			int samp = (int) ((fbuffer[i] * NORM_FLOAT_FACTOR) * (1 << 23));
// 			bufp[2] = (samp >> 16);
// 			bufp[1] = (samp >> 8);
// 			bufp[0] = (samp & 0xFF);
// 		}
// 		break;
// 
// 	default:
// 		assert(!"rtwritesamps: unknown output data format!");
// 		break;/*NOTREACHED*/
// 	}
//	return _convertBuffer;
}

static const float kFloatNormalizer = (1.0 / 32768.0);
typedef unsigned char *UCharP;

// Float to other, interleaved to interleaved converters

static void _convertIFloatToIBShort(void *in, void *out, int chans, int frames)
{
	float *fin = (float *) in;
	short *sout = (short *) out;
	int samps = chans * frames;
	for (int s = 0; s < samps; ++s)
		m_set_big_endian_short((UCharP)&sout[s], (short) fin[s]);	
}

static void _convertIFloatToILShort(void *in, void *out, int chans, int frames)
{
	float *fin = (float *) in;
	short *sout = (short *) out;
	int samps = chans * frames;
	for (int s = 0; s < samps; ++s)
		m_set_little_endian_short((UCharP)&sout[s], (short) fin[s]);	
}

static void _convertIFloatToIBFloat(void *in, void *out, int chans, int frames)
{
	float *fin = (float *) in;
	float *fout = (float *) out;
	int samps = chans * frames;
	for (int s = 0; s < samps; ++s)
		m_set_big_endian_float((UCharP)&fout[s], fin[s]);	
}

static void _convertIFloatToILFloat(void *in, void *out, int chans, int frames)
{
	float *fin = (float *) in;
	float *fout = (float *) out;
	int samps = chans * frames;
	for (int s = 0; s < samps; ++s)
		m_set_little_endian_float((UCharP)&fout[s], fin[s]);	
}

// Float to other, non-interleaved to interleaved converters

static void _convertNFloatToIBShort(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	for (int ch = 0; ch < chans; ++ch) {
		float *fbuffer = fin[ch];
		short *sout = ((short *) out) + ch;
		for (int fr = 0; fr < frames; ++fr, sout += chans) {
			m_set_big_endian_short((UCharP)sout, (short) fbuffer[fr]);
		}
	}
}

static void _convertNFloatToILShort(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	for (int ch = 0; ch < chans; ++ch) {
		float *fbuffer = fin[ch];
		short *sout = ((short *) out) + ch;
		for (int fr = 0; fr < frames; ++fr, sout += chans) {
			m_set_little_endian_short((UCharP)sout, (short) fbuffer[fr]);
		}
	}
}

static void _convertNFloatToIBFloat(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	for (int ch = 0; ch < chans; ++ch) {
		float *fbuffer = fin[ch];
		float *fout = ((float *) out) + ch;
		for (int fr = 0; fr < frames; ++fr, fout += chans) {
			m_set_big_endian_float((UCharP)fout, fbuffer[fr]);
		}
	}
}

static void _convertNFloatToILFloat(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	for (int ch = 0; ch < chans; ++ch) {
		float *fbuffer = fin[ch];
		float *fout = ((float *) out) + ch;
		for (int fr = 0; fr < frames; ++fr, fout += chans) {
			m_set_little_endian_float((UCharP)fout, fbuffer[fr]);
		}
	}
}

// Float to other, interleaved to non-interleaved converters

static void _convertIFloatToNBShort(void *in, void *out, int chans, int frames)
{
	float *fin = (float *) in;
	short **sout = (short **) out;
	for (int ch = 0; ch < chans; ++ch) {
		float *fbuffer = &fin[ch];
		short *sbuffer = sout[ch];
		for (int fr = 0; fr < frames; ++fr, fbuffer += chans) {
			m_set_big_endian_short((UCharP)&sbuffer[fr], (short) *fbuffer);
		}
	}
}

static void _convertIFloatToNLShort(void *in, void *out, int chans, int frames)
{
	float *fin = (float *) in;
	short **sout = (short **) out;
	for (int ch = 0; ch < chans; ++ch) {
		float *fbuffer = &fin[ch];
		short *sbuffer = sout[ch];
		for (int fr = 0; fr < frames; ++fr, fbuffer += chans) {
			m_set_little_endian_short((UCharP)&sbuffer[fr], (short) *fbuffer);
		}
	}
}

static void _convertIFloatToNBFloat(void *in, void *out, int chans, int frames)
{
	float *fin = (float *) in;
	float **fout = (float **) out;
	for (int ch = 0; ch < chans; ++ch) {
		float *finbuffer = &fin[ch];
		float *foutbuffer = fout[ch];
		for (int fr = 0; fr < frames; ++fr, finbuffer += chans) {
			m_set_big_endian_float((UCharP)&foutbuffer[fr], *finbuffer);
		}
	}
}

static void _convertIFloatToNLFloat(void *in, void *out, int chans, int frames)
{
	float *fin = (float *) in;
	float **fout = (float **) out;
	for (int ch = 0; ch < chans; ++ch) {
		float *finbuffer = &fin[ch];
		float *foutbuffer = fout[ch];
		for (int fr = 0; fr < frames; ++fr, finbuffer += chans) {
			m_set_little_endian_float((UCharP)&foutbuffer[fr], *finbuffer);
		}
	}
}

// Float to other, non-interleaved to non-interleaved converters

static void _convertNFloatToNBShort(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	short **sout = (short **) out;
	for (int ch = 0; ch < chans; ++ch) {
		float *fbuffer = fin[ch];
		short *sbuffer = sout[ch];
		for (int fr = 0; fr < frames; ++fr) {
			m_set_big_endian_short((UCharP)&sbuffer[fr], (short) fbuffer[fr]);
		}
	}
}

static void _convertNFloatToNLShort(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	short **sout = (short **) out;
	for (int ch = 0; ch < chans; ++ch) {
		float *fbuffer = fin[ch];
		short *sbuffer = sout[ch];
		for (int fr = 0; fr < frames; ++fr) {
			m_set_little_endian_short((UCharP)&sbuffer[fr], (short) fbuffer[fr]);
		}
	}
}

// Note: For now, because we know that non-interleaved floats are ALWAYS
// destined for audio HW, and all known audio HW that takes floats, takes
// them normalized, we normalize here and in the next function.

static void _convertNFloatToNBFloat(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	float **fout = (float **) out;
	for (int ch = 0; ch < chans; ++ch) {
		float *finbuffer = fin[ch];
		float *foutbuffer = fout[ch];
		for (int fr = 0; fr < frames; ++fr) {
			m_set_big_endian_float((UCharP)&foutbuffer[fr], 
								   finbuffer[fr] * kFloatNormalizer);
		}
	}
}

static void _convertNFloatToNLFloat(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	float **fout = (float **) out;
	for (int ch = 0; ch < chans; ++ch) {
		float *finbuffer = fin[ch];
		float *foutbuffer = fout[ch];
		for (int fr = 0; fr < frames; ++fr) {
			m_set_little_endian_float((UCharP)&foutbuffer[fr],
									  finbuffer[fr] * kFloatNormalizer);
		}
	}
}


// Short to float, interleaved to non-interleaved converters.

// This one is used only for record.

static void _convertIShortToNFloat(void *in, void *out, int chans, int frames)
{
	short *sin = (short *) in;
	float **fout = (float **) out;
	for (int ch = 0; ch < chans; ++ch) {
		short *sbuffer = &sin[ch];
		float *fbuffer = fout[ch];
		for (int fr = 0; fr < frames; ++fr, sbuffer += chans) {
			fbuffer[fr] = (float) *sbuffer;
		}
	}
}

// Conversion functions are assigned in pairs, regardless of rec/pb state.

int AudioDeviceImpl::setConvertFunctions(int frameFormat,
										 int deviceFormat,
										 bool frameInterleaved,
										 bool deviceInterleaved)
{
	_recConvertFunction = NULL;
	_playConvertFunction = NULL;
	switch (deviceFormat) {
	case MUS_BFLOAT:	// float frame, BE float device
		if (frameInterleaved) {
			if (deviceInterleaved) {
				_recConvertFunction = _convertIFloatToIBFloat;
				_playConvertFunction = _convertIFloatToIBFloat;
			}
			else {
				_recConvertFunction = _convertNFloatToIBFloat;
				_playConvertFunction = _convertIFloatToNBFloat;
			}
		}
		else {
			if (deviceInterleaved) {
				_recConvertFunction = _convertIFloatToNBFloat;
				_playConvertFunction = _convertNFloatToIBFloat;
			}
			else {
				_recConvertFunction = _convertNFloatToNBFloat;
				_playConvertFunction = _convertNFloatToNBFloat;
			}
		}
		break;
		
	case MUS_LFLOAT:	// float frame, LE float device
		if (frameInterleaved) {
			if (deviceInterleaved) {
				_recConvertFunction = _convertIFloatToILFloat;
				_playConvertFunction = _convertIFloatToILFloat;
			}
			else {
				_recConvertFunction = _convertNFloatToILFloat;
				_playConvertFunction = _convertIFloatToNLFloat;
			}
		}
		else {
			if (deviceInterleaved) {
				_recConvertFunction = _convertIFloatToNLFloat;
				_playConvertFunction = _convertNFloatToILFloat;
			}
			else {
				_recConvertFunction = _convertNFloatToNLFloat;
				_playConvertFunction = _convertNFloatToNLFloat;
			}
		}
		break;

	case MUS_BSHORT:	// float frame, BE short device
		if (frameInterleaved) {
			if (deviceInterleaved) {
				_playConvertFunction = _convertIFloatToIBShort;
			}
			else {
				_playConvertFunction = _convertIFloatToNBShort;
			}
		}
		else {
			if (deviceInterleaved) {
				_playConvertFunction = _convertNFloatToIBShort;
			}
			else {
				_playConvertFunction = _convertNFloatToNBShort;
			}
		}
		break;

	case MUS_LSHORT:	// float frame, LE short device
		if (frameInterleaved) {
			if (deviceInterleaved) {
				_playConvertFunction = _convertIFloatToILShort;
			}
			else {
				_playConvertFunction = _convertIFloatToNLShort;
			}
		}
		else {
			if (deviceInterleaved) {
				_recConvertFunction = _convertIShortToNFloat;
				_playConvertFunction = _convertNFloatToILShort;
			}
			else {
				_playConvertFunction = _convertNFloatToNLShort;
			}
		}
		break;

	default:
		break;
	}
	if (isPlaying() && _playConvertFunction == NULL)
		return error("This format conversion is currently not supported!");
	if (isRecording() && _recConvertFunction == NULL)
		return error("This format conversion is currently not supported!");
	return 0;
}

