// AudioDeviceImpl.cpp

#include "AudioDeviceImpl.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

// AudioDeviceImpl is the "workhorse" intermediate base class for most
// AudioDevice leaf classes.  It holds the common state, handles all logic
// that can be considered common to all AudioDevices (including state checking
// and format conversion), and handles the callbacks associated with playback
// and stop.  All primary AudioDevice methods are implemented in this class,
// where they in turn call the protected virtual "hook" functions, which
// perform the HW-specific operations.

AudioDeviceImpl::AudioDeviceImpl() 
	: _mode(Unset), _state(Closed),
	  _frameFormat(MUS_UNSUPPORTED), _deviceFormat(MUS_UNSUPPORTED),
	  _frameChannels(0), _deviceChannels(0), _samplingRate(0.0), _maxFrames(0),
	  _runCallback(NULL), _stopCallback(NULL),
	  _convertBuffer(NULL), 
	  _recConvertFunction(NULL), _playConvertFunction(NULL)
{
}

AudioDeviceImpl::~AudioDeviceImpl()
{
	/* if this asserts, close() is not being called before destructor */
	assert(_convertBuffer == NULL);
	delete _runCallback;
	delete _stopCallback;
}

int	AudioDeviceImpl::setFrameFormat(int frameSampFmt, int frameChans)
{
	_frameFormat = frameSampFmt;
	_frameChannels = frameChans;
	return 0;
}

int AudioDeviceImpl::open(int mode, int sampfmt, int chans, double srate)
{
	printf("AudioDeviceImpl::open: opening device 0x%x for %s in %s mode\n",
		   this,
		   (mode & DirectionMask) == RecordPlayback ? "Record/Playback"
		   : (mode & DirectionMask) == Playback ? "Playback" : "Record",
		   (mode & Passive) ? "Passive" : "Active");
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
#ifdef ORIGINAL_CODE
int AudioDeviceImpl::start(AudioDevice::Callback *callback)
{
	int status = 0;
	if (!isRunning()) {
		_runCallback = callback;
		State oldState = getState();
		setState(Running);
		if ((status = doStart()) != 0) {
			setState(oldState);
		}
	}
	return status;
}
#else
int AudioDeviceImpl::start(AudioDevice::Callback *callback)
{
	int status = 0;
	if (!isRunning()) {
		_runCallback = callback;
		if ((status = doStart()) == 0) {
			setState(Running);
		}
	}
	return status;
}
#endif

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

int AudioDeviceImpl::setStopCallback(Callback *stopCallback)
{
	if (stopCallback != _stopCallback) {
		delete _stopCallback;
		_stopCallback = stopCallback;
	}
	return 0;
}

bool AudioDeviceImpl::runCallback()
{ 
	assert(_runCallback);
	return _runCallback->call();
}

// Call the callback if present, then delete and null to avoid multiple
// calls

bool AudioDeviceImpl::stopCallback() {
	bool ret = (_stopCallback) ? _stopCallback->call() : false;
	delete _stopCallback;
	_stopCallback = NULL;
	return ret;
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
	delete _runCallback;
	_runCallback = NULL;
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
	else if (IS_24BIT_FORMAT(fmt)) {
		buffer = (void *) new char[chans * len * 3];
	}
	else {
		error("createInterleavedBuffer: unknown sample format!");
		return NULL;
	}
	if (!buffer)
		error("createInterleavedBuffer: memory allocation failure");
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
	else if (IS_24BIT_FORMAT(fmt)) {
		delete [] (char *) _convertBuffer;
	}
}

// Code for creating and destroying non-interleaved conversion buffer

template <class Type> 
Type **newNoninterleavedBuffer(int chans, int len) {
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

template <class Type> 
void deleteNoninterleavedBuffer(void *buf, int chans) {
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
	else if (IS_24BIT_FORMAT(fmt)) {
		buffer = (void *) newNoninterleavedBuffer<char>(chans, len * 3);
	}
	else if (IS_SHORT_FORMAT(fmt)) {
		buffer = (void *) newNoninterleavedBuffer<short>(chans, len);
	}
	else {
		error("createNoninterleavedBuffer: unsupported sample format!");
		return NULL;
	}
	if (!buffer)
		error("createNoninterleavedBuffer: memory allocation failure");
	
	return buffer;
}

void
AudioDeviceImpl::destroyNoninterleavedBuffer(int fmt, int chans)
{
	if (IS_FLOAT_FORMAT(fmt)) {
		deleteNoninterleavedBuffer<float>(_convertBuffer, chans);
	}
	else if (IS_24BIT_FORMAT(fmt)) {
		deleteNoninterleavedBuffer<char>(_convertBuffer, chans);
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
			return -1;
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
		_maxFrames = reqWriteSize * reqCount;
		return setupConversion();
	}
	*pWriteSize = -1;	// error condition
	return -1;
}

int AudioDeviceImpl::setupConversion()
{
	int status = createConvertBuffer(_maxFrames);
	if (status == 0) {
		// Hand in the raw format because the accessor functions
		// filter out the interleave and normalize bits.
		status = setConvertFunctions(_frameFormat, _deviceFormat);
	}
	return status;
}

// This is used by specialized derived classes that need to change formats
// in mid-stream.

int AudioDeviceImpl::resetFormatConversion()
{
	destroyConvertBuffer();
	return setupConversion();
}

int AudioDeviceImpl::getDeviceBytesPerFrame() const
{
	return mus_data_format_to_bytes_per_sample(getDeviceFormat()) 
				* getDeviceChannels();
}


void *
AudioDeviceImpl::convertFrame(void *inbuffer, void *outbuffer,
							  int frames, bool recording)
{
	const int chans = getFrameChannels();
		
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
}

static const float kFloatNormalizer = (1.0 / 32768.0);
typedef unsigned char *UCharP;

// Template functions for lowest-level -toType conversion,

template <class OutType>
inline void setLE(OutType *pOut, float in);

template <class OutType>
inline void setBE(OutType *pOut, float in);

// Specializations of the above.

template <>
inline void setLE<float>(float *pOut, float in)
{
	m_set_little_endian_float((UCharP) pOut, in);	
}

template <>
inline void setBE<float>(float *pOut, float in)
{
	m_set_big_endian_float((UCharP) pOut, in);	
}

template <>
inline void setLE<short>(short *pOut, float in)
{
	m_set_little_endian_short((UCharP) pOut, in);	
}

template <>
inline void setBE<short>(short *pOut, float in)
{
	m_set_big_endian_short((UCharP) pOut, in);	
}

#ifdef USING_INTERLEAVED_BUFFERS

//
// Interleaved to interleaved converters
//

template <class Intype, class OutType>
static void _convertIToIBig(void *in, void *out, int chans, int frames)
{
	Intype *tin = (Intype *) in;
	OutType *tout = (OutType *) out;
	const int samps = chans * frames;
	for (int s = 0; s < samps; ++s)
		setBE(&tout[s], tin[s]);	
}

template <class Intype, class OutType>
static void _convertIToILittle(void *in, void *out, int chans, int frames)
{
	Intype *tin = (Intype *) in;
	OutType *tout = (OutType *) out;
	int samps = chans * frames;
	for (int s = 0; s < samps; ++s)
		setLE(&tout[s], tin[s]);	
}

// Note:  The 24bit converters are only used for writing soundfiles to disk.

template <class Type>
static void _convertIToIB24Bit(void *in, void *out, int chans, int frames)
{
	Type *tin = (Type *) in;
	unsigned char *cout = (unsigned char *) out;
	int samps = chans * frames;
	for (int s = 0; s < samps; ++s, cout += 3) {
		const int samp = (int) (tin[s] * (1 << 8));
		cout[0] = (samp >> 16);
		cout[1] = (samp >> 8);
		cout[2] = (samp & 0xFF);
	}
}

template <class Type>
static void _convertIToIL24Bit(void *in, void *out, int chans, int frames)
{
	Type *tin = (Type *) in;
	unsigned char *cout = (unsigned char *) out;
	int samps = chans * frames;
	for (int s = 0; s < samps; ++s, cout += 3) {
		const int samp = (int) (tin[s] * (1 << 8));
		cout[2] = (samp >> 16);
		cout[1] = (samp >> 8);
		cout[0] = (samp & 0xFF);
	}
}

// These two are for writing normalized floats to file (or for interleaved HW
// which supports normalized floats).

static void _convertIFloatToIBFloatNormalized(void *in, void *out, int chans, int frames)
{
	float *fin = (float *) in;
	float *fout = (float *) out;
	int samps = chans * frames;
	for (int s = 0; s < samps; ++s)
		m_set_big_endian_float((UCharP)&fout[s], fin[s] * kFloatNormalizer);	
}

static void _convertIFloatToILFloatNormalized(void *in, void *out, int chans, int frames)
{
	float *fin = (float *) in;
	float *fout = (float *) out;
	int samps = chans * frames;
	for (int s = 0; s < samps; ++s)
		m_set_little_endian_float((UCharP)&fout[s], fin[s] * kFloatNormalizer);	
}

#endif // USING_INTERLEAVED_BUFFERS

//
// Non-interleaved to interleaved converters.
//

template <class Intype, class OutType>
static void _convertNToIBig(void *in, void *out, int chans, int frames)
{
	Intype **tin = (Intype **) in;
	for (int ch = 0; ch < chans; ++ch) {
		Intype *tbuffer = tin[ch];
		OutType *tout = ((OutType *) out) + ch;
		for (int fr = 0; fr < frames; ++fr, tout += chans) {
			setBE(tout, tbuffer[fr]);
		}
	}
}

template <class Intype, class OutType>
static void _convertNToILittle(void *in, void *out, int chans, int frames)
{
	Intype **tin = (Intype **) in;
	for (int ch = 0; ch < chans; ++ch) {
		Intype *tbuffer = tin[ch];
		OutType *tout = ((OutType *) out) + ch;
		for (int fr = 0; fr < frames; ++fr, tout += chans) {
			setLE(tout, tbuffer[fr]);
		}
	}
}

// Note:  The 24bit converters are only used for writing soundfiles to disk.

static void _convertNFloatToIB24Bit(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	for (int ch = 0; ch < chans; ++ch) {
		float *fbuffer = fin[ch];
		unsigned char *cout = ((unsigned char *) out) + (ch * 3);
		int incr = chans * 3;
		for (int fr = 0; fr < frames; ++fr, cout += incr) {
			const int samp = (int) (fbuffer[fr] * (1 << 8));
			cout[0] = (samp >> 16);
			cout[1] = (samp >> 8);
			cout[2] = (samp & 0xFF);
		}
	}
}

static void _convertNFloatToIL24Bit(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	for (int ch = 0; ch < chans; ++ch) {
		float *fbuffer = fin[ch];
		unsigned char *cout = ((unsigned char *) out) + (ch * 3);
		int incr = chans * 3;
		for (int fr = 0; fr < frames; ++fr, cout += incr) {
			int samp = (int) (fbuffer[fr] * (1 << 8));
			cout[2] = (samp >> 16);
			cout[1] = (samp >> 8);
			cout[0] = (samp & 0xFF);
		}
	}
}

static void _convertNFloatToIBFloatNormalized(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	for (int ch = 0; ch < chans; ++ch) {
		float *fbuffer = fin[ch];
		float *fout = ((float *) out) + ch;
		for (int fr = 0; fr < frames; ++fr, fout += chans) {
			m_set_big_endian_float((UCharP)fout, fbuffer[fr] * kFloatNormalizer);
		}
	}
}

static void _convertNFloatToILFloatNormalized(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	for (int ch = 0; ch < chans; ++ch) {
		float *fbuffer = fin[ch];
		float *fout = ((float *) out) + ch;
		for (int fr = 0; fr < frames; ++fr, fout += chans) {
			m_set_little_endian_float((UCharP)fout, fbuffer[fr] * kFloatNormalizer);
		}
	}
}

//
// Interleaved to non-interleaved converters
//

template <class Intype, class OutType>
static void _convertIToN(void *in, void *out, int chans, int frames)
{
	Intype *tin = (Intype *) in;
	OutType **tout = (OutType **) out;
	for (int ch = 0; ch < chans; ++ch) {
		Intype *ibuffer = &tin[ch];
		OutType *obuffer = tout[ch];
		for (int fr = 0; fr < frames; ++fr, ibuffer += chans) {
			obuffer[fr] = (OutType) *ibuffer;
		}
	}
}

static void _convertIFloatNormalizedToNFloat(void *in, void *out, int chans, int frames)
{
	float *fin = (float *) in;
	float **fout = (float **) out;
	for (int ch = 0; ch < chans; ++ch) {
		float *ibuffer = &fin[ch];
		float *obuffer = fout[ch];
		for (int fr = 0; fr < frames; ++fr, ibuffer += chans) {
			obuffer[fr] = *ibuffer * 32768.0;
		}
	}
}

//
// Non-interleaved to non-interleaved converters
//

template <class Intype, class OutType>
static void _convertNToN(void *in, void *out, int chans, int frames)
{
	Intype **tin = (Intype **) in;
	OutType **tout = (OutType **) out;
	for (int ch = 0; ch < chans; ++ch) {
		Intype *ibuffer = tin[ch];
		OutType *obuffer = tout[ch];
		for (int fr = 0; fr < frames; ++fr) {
			obuffer[fr] = (OutType) ibuffer[fr];
		}
	}
}

// Note: For now, because we know that non-interleaved floats are ALWAYS
// from or to audio HW, and all known audio HW that takes floats, takes
// them normalized, we only need one function in cases where normalizing
// needs to be done, and one for de-normalizing.

static void _convertNFloatToNFloatNormalized(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	float **fout = (float **) out;
	for (int ch = 0; ch < chans; ++ch) {
		float *finbuffer = fin[ch];
		float *foutbuffer = fout[ch];
		for (int fr = 0; fr < frames; ++fr) {
			foutbuffer[fr] = (finbuffer[fr] * kFloatNormalizer);
		}
	}
}

template <class OutType>
static void _convertNFloatNormalizedToN(void *in, void *out, int chans, int frames)
{
	float **fin = (float **) in;
	OutType **tout = (OutType **) out;
	for (int ch = 0; ch < chans; ++ch) {
		float *finbuffer = fin[ch];
		float *toutbuffer = tout[ch];
		for (int fr = 0; fr < frames; ++fr) {
			toutbuffer[fr] = OutType(finbuffer[fr] * 32768.0f);
		}
	}
}


// Conversion functions are assigned in pairs, regardless of rec/pb state.

int AudioDeviceImpl::setConvertFunctions(int rawFrameFormat,
										 int rawDeviceFormat)
{
	const bool frameNormalized = IS_NORMALIZED_FORMAT(rawFrameFormat);
	const bool frameInterleaved = IS_INTERLEAVED_FORMAT(rawFrameFormat);
	const bool deviceNormalized = IS_NORMALIZED_FORMAT(rawDeviceFormat);
	const bool deviceInterleaved = IS_INTERLEAVED_FORMAT(rawDeviceFormat);
	
	// For simplicity's sake, this routine does not account for interleaved
	// frames passed in from RTcmix, because we never do that anymore.  If
	// this code is used in other applications, the if (frameInterleaved)
	// blocks can be added back in.  -DS
	
	assert(!frameInterleaved);
	
	_recConvertFunction = NULL;
	_playConvertFunction = NULL;
	
	// The device may be a file or HW, so take endian-ness into account for
	// playback and some record options.
	if (deviceInterleaved) {
		switch (MUS_GET_FORMAT(rawDeviceFormat)) {
		case MUS_LFLOAT:	// float frame, LE float device
			if (deviceNormalized == frameNormalized) {
				_recConvertFunction = _convertIToN<float, float>;
				_playConvertFunction = _convertNToILittle<float, float>;
			}
			else if (deviceNormalized && !frameNormalized) {
				_recConvertFunction = _convertIFloatNormalizedToNFloat;
				_playConvertFunction = _convertNFloatToILFloatNormalized;
			}
			break;

		case MUS_BFLOAT:	// float frame, BE float device
			if (deviceNormalized == frameNormalized) {
				_recConvertFunction = _convertIToN<float, float>;
				_playConvertFunction = _convertNToIBig<float, float>;
			}
			else if (deviceNormalized && !frameNormalized) {
				_recConvertFunction = _convertIFloatNormalizedToNFloat;
				_playConvertFunction = _convertNFloatToIBFloatNormalized;
			}
			break;

		case MUS_B24INT:	// float frame, BE 24bit file.  HW record not supported
			_playConvertFunction = _convertNFloatToIB24Bit;
			break;

		case MUS_L24INT:	// float frame, BE 24bit file.  HW record not supported
			_playConvertFunction = _convertNFloatToIL24Bit;
			break;

		case MUS_LSHORT:	// float frame, LE short device
			_recConvertFunction = _convertIToN<short, float>;
			_playConvertFunction = _convertNToILittle<float, short>;
			break;

		case MUS_BSHORT:	// float frame, BE short device
			_recConvertFunction = _convertIToN<short, float>;
			_playConvertFunction = _convertNToIBig<float, short>;
			break;
			
		default:
			break;
		}
	}
	// Non-interleaved devices are always local HW, so no endian issues.
	else {
		switch (MUS_GET_FORMAT(rawDeviceFormat)) {
		case MUS_LFLOAT:	// float frame, BE float device
		case MUS_BFLOAT:	// float frame, BE float device
			if (deviceNormalized == frameNormalized) {
				_recConvertFunction = _convertNToN<float, float>;
				_playConvertFunction = _convertNToN<float, float>;
			}
			else if (deviceNormalized && !frameNormalized) {
				_recConvertFunction = _convertNFloatNormalizedToN<float>;
				_playConvertFunction = _convertNFloatToNFloatNormalized;
			}
			break;

		case MUS_LSHORT:
		case MUS_BSHORT:
			_recConvertFunction = _convertNToN<short, float>;
			_playConvertFunction = _convertNToN<float, short>;
			break;

		default:
			break;
		}
	}
	
	if (isPlaying() && _playConvertFunction == NULL)
		return error("This format conversion is currently not supported!");
	if (isRecording() && _recConvertFunction == NULL)
		return error("This format conversion is currently not supported!");
	return 0;
}

