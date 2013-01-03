/* CONVOLVE1 - convolution instrument

      p0 = output start time
      p1 = input start time
      p2 = input duration
      p3 = amplitude multiplier
      p4 = impulse response table (use maketable("soundfile", ...))
           must be mono; maketable lets you select channel.
      p5 = impulse start time (time to skip in table)
      p6 = impulse duration
      p7 = impulse gain
      p8 = window function table
      p9 = wet percent (0-1)
      p10 = input channel
      p11 = pan (in percent-to-left form)

   p3 (amplitude), p9 (wet percent) and p11 (pan) can receive dynamic updates
   from a table or real-time control source.

   This instrument is designed for short impulse responses: a maximum of about
   6 seconds at 44.1kHz sampling rate, though a few hundred milliseconds is more
   typical.  The impulse response duration determines the FFT size: the longer
   the response, the larger the FFT size, and therefore the more transient
   smearing.  Only one FFT of the impulse response is taken.  The FFT size of
   the input equals that of the impulse response, and there is a delay before
   the start of sound approximately equal to the impulse response duration.
   (Actually, the delay in frames is the smallest power of two that is greater
   than the impulse response length.)

   With this in mind, the best strategy for using the instrument is to loop with
   very short notes, while creeping through both the input and the impulse
   response.  See the example scores.

   NOTE: This instrument can work with very small buffer sizes (e.g., 32), but
   it probably will cause audio buffer underruns if the impulse response is
   longer than about 20 milliseconds.

   John Gibson, 5/31/05 (based on cmix convolve)
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Ougens.h>
#include <PField.h>
#include "CONVOLVE1.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG

#ifdef DEBUG
   #define DPRINT(msg)                    printf((msg))
   #define DPRINT1(msg, arg)              printf((msg), (arg))
   #define DPRINT2(msg, arg1, arg2)       printf((msg), (arg1), (arg2))
#else
   #define DPRINT(msg)
   #define DPRINT1(msg, arg)
   #define DPRINT2(msg, arg1, arg2)
#endif

const int kMinFFTsize = 256;
const int kMaxImpulseFrames = 262144;	// 2^18: about 6 seconds at 44100

inline int imax(int x, int y) { return x > y ? x : y; }
inline int imin(int x, int y) { return x < y ? x : y; }

inline float clamp(float low, float val, float high)
{
	if (val < low)
		return low;
	else if (val > high)
		return high;
	return val;
}


CONVOLVE1::CONVOLVE1()
	: _branch(0),
	  _inbuf(NULL),      // buffer to read (possibly multichannel) input
	  _imp(NULL),        // buffer holding impulse response spectrum
	  _ovadd(NULL),      // buffer for overlap-adding fft output
	  _dry(NULL),        // dry signal for output
	  _wet(NULL),        // wet signal for output
	  _bucket(NULL),     // input bucket
	  _winosc(NULL)      // window function table oscillator
{
}

CONVOLVE1::~CONVOLVE1()
{
	delete [] _inbuf;
	delete [] _imp;
	delete [] _ovadd;
	delete [] _dry;
	delete [] _wet;
	delete _winosc;
	delete _bucket;
	delete _fft;
}

int CONVOLVE1::init(double p[], int n_args)
{
	const float outskip = p[0];
	const float inskip = p[1];
	const float indur = p[2];
	const float impskip = p[5];
	const float impdur = p[6];
	if (impdur <= 0.0)
		return die("CONVOLVE1", "Impulse duration must be greater than zero.");
	_impgain = p[7];
	_wetpct = p[9];	// NB: used before first call to doupdate
	if (_wetpct < 0.0 || _wetpct > 1.0)
		return die("CONVOLVE1", "Wet percent must be between 0 and 1.");
	_inchan = int(p[10]);

	// Read impulse response table, and set FFT size based on this.
	_imptab = (double *) getPFieldTable(4, &_imptablen);
	if (_imptab == NULL)
		return die("CONVOLVE1", "Must store impulse response in a table.");
	_impStartIndex = int(impskip * SR + 0.5);
	if (_impStartIndex >= _imptablen)
		return die("CONVOLVE1", "Impulse start time exceeds impulse duration.");
	int impend = _impStartIndex + int(impdur * SR + 0.5);
	// NOTE: <impend> may be past end of table; we handle that in prepareImpulse.
	DPRINT2("impend=%d, _imptablen=%d\n", impend, _imptablen);
	_impframes = impend - _impStartIndex;

	_halfFFTlen = kMinFFTsize / 2;
	for ( ; _halfFFTlen < kMaxImpulseFrames; _halfFFTlen *= 2)
		if (_halfFFTlen >= _impframes)
			break;
	_fftlen = 2 * _halfFFTlen;
	DPRINT2("_impframes=%d, _halfFFTlen=%d\n", _impframes, _halfFFTlen);
	rtcmix_advise("CONVOLVE1", "Using %d impulse response frames.  FFT length is %d.",
				_impframes, _fftlen);

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;	// no input
	if (_inchan >= inputChannels())
		return die("CONVOLVE1", "You asked for channel %d of a %d-channel input.",
										_inchan, inputChannels());

	// Latency is the delay before the FFT looks at actual input rather than
	// zero-padding.  Need to let inst run long enough to compensate for this.

	const float latency = float(_impframes) / SR;
	const float ringdur = latency;
	if (rtsetoutput(outskip, latency + indur + ringdur, this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() > 2)
		return die("CONVOLVE1", "Must have mono or stereo output.");
	_inframes = int(indur * SR + 0.5);		// not including latency

	int winlen;
	double *wintab = (double *) getPFieldTable(8, &winlen);
	if (wintab) {
		if (winlen > 32767)	// limit for new fixed-point Ooscili
			return die("CONVOLVE1", "Window table size must be less than 32768.");
		const float freq = 1.0 / ((float) _impframes / SR);
		_winosc = new Ooscili(SR, freq, wintab, winlen);
	}

	return nSamps();
}


int CONVOLVE1::prepareImpulse()
{
	// copy and window impulse response; zero-pad to fft length
	const int end = imin(_imptablen, _impframes);
	if (_winosc) {
		for (int i = 0, j = _impStartIndex; i < end; i++, j++)
			_fftbuf[i] = _imptab[j] * _winosc->next(i);
	}
	else
		for (int i = 0, j = _impStartIndex; i < end; i++, j++)
			_fftbuf[i] = _imptab[j];
	for (int i = end; i < _fftlen; i++)
		_fftbuf[i] = 0.0f;

	_fft->r2c();	// take FFT

	// normalize spectrum
	double max = 0.0;
	for (int i = 0; i < _halfFFTlen; i++) {
		int index = i << 1;
		float a = _fftbuf[index];
		float b = _fftbuf[index + 1];
		double c2 = (a * a) + (b * b);
		if (c2 > max)
			max = c2;
	}
	if (max != 0.0)
		max = _impgain / sqrt(max);
	else
		return die("CONVOLVE1", "Impulse response is all zeros.");

	_imp = new float [_fftlen];
	if (_imp == NULL)
		return -1;
	for (int i = 0; i < _fftlen; i++)
		_imp[i] = _fftbuf[i] * max;

//	for (int i = 0; i < _fftlen; i++) printf("[%d] = %f\n", i, _imp[i]);

	return 0;
}


int CONVOLVE1::configure()
{
	_inbuf = new float [RTBUFSAMPS * inputChannels()];
	_ovadd = new float [_impframes];
	if (_inbuf == NULL || _ovadd == NULL)
		return -1;
	for (int i = 0; i < _impframes; i++)
		_ovadd[i] = 0.0f;

	// read index chases write index by _impframes
	_outframes = imax(_halfFFTlen, RTBUFSAMPS) * 2;
	_outReadIndex = _outframes - _impframes;
	_outWriteIndex = 0;
	_dry = new float [_outframes];
	_wet = new float [_outframes];
	if (_dry == NULL || _wet == NULL)
		return -1;
	for (int i = 0; i < _outframes; i++)
		_dry[i] = _wet[i] = 0.0f;
	DPRINT1("_outframes: %d\n", _outframes);

	_bucket = new Obucket(_impframes, processWrapper, (void *) this);
	if (_bucket == NULL)
		return -1;

	_fft = new Offt(_fftlen);
	if (_fft == NULL)
		return -1;
	_fftbuf = _fft->getbuf();

	if (prepareImpulse() != 0)
		return -1;

	return 0;
}


void CONVOLVE1::convolve()
{
	_fft->r2c();

	_fftbuf[0] = _fftbuf[0] * _imp[0];			// DC
	_fftbuf[1] = _fftbuf[1] * _imp[1];			// Nyquist

	int i = 2;
	int j = 3;
	for ( ; i < _fftlen; i += 2, j += 2) {
		float a = (_fftbuf[i] * _imp[i]) - (_fftbuf[j] * _imp[j]);
		float b = (_fftbuf[i] * _imp[j]) + (_fftbuf[j] * _imp[i]);
		_fftbuf[i] = a;
		_fftbuf[j] = b;
	}

	_fft->c2r();
}


// Called by Obucket whenever the bucket is full (i.e., has _halfFFTlen samps).
// This static member wrapper function lets us call a non-static member
// function, which we can't pass directly as a callback to Obucket.
// See http://www.newty.de/fpt/callback.html for one explanation of this.

void CONVOLVE1::processWrapper(const float buf[], const int len, void *obj)
{
	CONVOLVE1 *myself = (CONVOLVE1 *) obj;
	myself->process(buf, len);
}


void CONVOLVE1::process(const float *buf, const int len)
{
	DPRINT1("CONVOLVE1::process (len=%d)\n", len);

	// NOTE: <len> will always be equal to _impframes
	if (_winosc)
		for (int i = 0; i < len; i++)
			_fftbuf[i] = buf[i] * _winosc->next(i);
	else
		for (int i = 0; i < len; i++)
			_fftbuf[i] = buf[i];
	for (int i = len; i < _fftlen; i++)
		_fftbuf[i] = 0.0f;

	convolve();

	for (int i = 0, j = len; i < len; i++, j++) {
		_ovadd[i] += _fftbuf[i];
		_wet[_outWriteIndex] = _ovadd[i];
		_dry[_outWriteIndex] = buf[i];
		incrementOutWriteIndex();
		_ovadd[i] = _fftbuf[j];
	}

//	for (int i = 0; i < _fftlen; i++) printf("[%d] = %f\n", i, _fftbuf[i]);
}


void CONVOLVE1::doupdate()
{
	double p[12];
	update(p, 12, 1 << 3 | 1 << 9 | 1 << 11);

	_amp = p[3];
	_wetpct = clamp(0.0f, p[9], 1.0f);
	_pan = p[11];
}


int CONVOLVE1::run()
{
	const int inchans = inputChannels();
	const int outchans = outputChannels();
	const int nframes = framesToRun();

	if (currentFrame() < _inframes) {
		const int insamps = nframes * inchans;
		rtgetin(_inbuf, this, insamps);
		for (int i = _inchan; i < insamps; i += inchans)
			_bucket->drop(_inbuf[i]);
	}
	else {
		for (int i = 0; i < nframes; i++)
			_bucket->drop(0.0f);
	}
	// If in last run invocation, make sure everything in bucket is processed.
	if (currentFrame() + nframes >= nSamps())
		_bucket->flush();

	float drypct = 1.0 - _wetpct;

	for (int i = 0; i < nframes; i++) {
		if (--_branch <= 0) {
			doupdate();
			drypct = 1.0 - _wetpct;
			_branch = getSkip();
		}

		float out[2];
		out[0] = (_wet[_outReadIndex] * _wetpct) + (_dry[_outReadIndex] * drypct);
		incrementOutReadIndex();
		out[0] *= _amp;

		if (outchans == 2) {
			out[1] = out[0] * (1.0 - _pan);
			out[0] *= _pan;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeCONVOLVE1()
{
	CONVOLVE1 *inst;

	inst = new CONVOLVE1();
	inst->set_bus_config("CONVOLVE1");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("CONVOLVE1", makeCONVOLVE1);
}
#endif
