#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <rt.h>
#include <rtdefs.h>
#include <string.h>
#include <assert.h>

#include "pv.h"
#include "PVOC.h"
#include "setup.h"
#include "PVFilter.h"

/* Christopher Penrose's Notes

Parameters!

Ok, you need to learn what I call the "Dolson Rule":

the greatest value of the D (decimation) and I (interpolation) values
should never be greater than N/8 if you wish to avoid gross amplitude
modulation.  D is the input or analysis overlap (overlap in samples =
N-D) while I is the output or resynthesis overlap (overlap in samples
= N-I).  The ratio between D and I determines time scaling.  If D/I is
greater than 1, then the sound will have a shorter duration.  If D/I
is less than 1, then the sound will be longer.

Remember that frequency resolution is nyquist frequency/(N/2).  Thus,
if you want to represent noisy signals (at 44.1KHz), then window sizes
(N) of 4096 are not uncommon.  Window size must always be a power of
two.  Also, decimation (D) determines the sampling rate of analysis,
so for large values of N, it is typical to use overlap values (maximum
of D and I) <= 128.

 */

#undef debug

#ifdef MACOSX
	#define cosf(x) (float) cos((double)(x))
	#define sinf(x) (float) sin((double)(x))
	#define hypotf(x, y) (float) hypot((double)(x), (double)(y))
	#define atan2f(y, x) (float) atan2((double)(y), (double)(x))
#endif
	

const complex zero = { 0., 0. };
const complex one = { 1., 0. };
float TWOPI;

inline int min(int x, int y) { return (x <= y) ? x : y; }
inline int max(int x, int y) { return (x >= y) ? x : y; }

// static float maxof( float *a, int n )
// {
//  register float *lim = a + n, m;
//	 for ( m = *a++; a < lim; a++ )
// 	if ( *a > m )
// 		m = *a;
//	 return( m );
// }

// static float maxamp( float *in, int n, int per, int ix, int sz )
// {
// 	 float *lim = in + n;
// 	 int i, found;
// 	 static float **buf, *mx;
// 	 static int *ptr;
// 	 register int *p;
// 	 register float *b, *m;
// 
//	 if ( _first ) {
// 		_first = 0;
// 		buf = (float **) malloc( sz*sizeof(float *) );
// 		for ( i = 0; i < sz; i++ )
// 			buf[i] = (float *) calloc( per, sizeof(float) );
// 		ptr = (int *) calloc( sz, sizeof(int) );
// 		mx = (float *) calloc( sz, sizeof(float) );
//	 }
//	 --ix;
//	 for (
// 	b = buf[ix] , p = &ptr[ix] , m = &mx[ix], found = 0;
// 	in < lim;
// 	in++
//	 ) {
// 	if ( ( *(b+*p) = *in >= 0. ? *in : -*in ) >= *m )
// 		found = *m = *(b+*p);
// 	if ( ++(*p) > per )
// 		*p = 0;
//	 }
//	 if ( found )
// 	return( *m );
//	 else
// 	return( *m = maxof( b, per ) );
// }

static void vvmult( float *out, float *a, float *b, int n )
{
	register float *lim = out + n;
	while ( out < lim )
		*out++ = *a++ * *b++;
}


/* PVOC: phase vocoder instrument
*
*  p0 = outskip
*  p1 = inskip
*  p2 = dur
*  p3 = AMP (dynamic)
*  p4 = input channel
*  p5 = fft size
*  p6 = window size
*  p7 = decimation amount (READIN) (dynamic)
*  p8 = interpolation amount (putout)
*  p9 = pitch multiplier (dynamic)
*  p10 = npoles
*  p11 = oscillator threshold (dynamic)
*
*/

PVOC::PVOC() : _first(1), _valid(-1)
{
	obank = 0;
	_inReadOffset = _inWriteOffset = _outReadOffset = _outWriteOffset = 0;
	_cachedInFrames = 0;
	_cachedOutFrames = 0;
	_inbuf = NULL;
	_outbuf = NULL;
	_pvFilter = NULL;
	Wanal= NULL;
	Wsyn= NULL;
	_pvInput= NULL;
	Hwin= NULL;
	winput= NULL;
	lpcoef= NULL;
	_fftBuf= NULL;
	channel= NULL;
	_pvOutput= NULL;
	_convertPhase = NULL;
	_unconvertPhase = NULL;
	_lastAmp = NULL;
	_lastFreq = NULL;
	_index = NULL;
	_table = NULL;
}

PVOC::~PVOC()
{
	delete [] _convertPhase;
	delete [] _unconvertPhase;
	delete [] Wanal;
	delete [] Wsyn;
	delete [] _pvInput;
	delete [] Hwin;
	delete [] winput;
	RefCounted::unref(_pvFilter);
	delete [] lpcoef;
	delete [] _fftBuf;
	delete [] channel;
	delete [] _pvOutput;
	delete [] _inbuf;
	delete [] _outbuf;
	delete [] _lastAmp;
	delete [] _lastFreq;
	delete [] _index;
	delete [] _table;
}

inline float *
NewArray(int size)
{
	float *arr = new float[size];
	if (!arr) {
		die("PVOC", "Unable to allocate memory");
        rtOptionalThrow(MEMORY_ERROR);
		return NULL;
	}
	memset(arr, 0, size * sizeof(float));
	return arr;
}

int PVOC::init(double *p, int n_args)
{
	if (!n_args || n_args < 9) {
		die("PVOC",
		"usage:\nPVOC(outskip, inskip, dur, amp, input_chan, fft_size, window_size, decim, interp, [ pitch_mult, npoles, osc threshold ])");
		return(DONT_SCHEDULE);
	}
	if (outputchans != 1) {
		die("PVOC", "Output file must have 1 channel only");
		return(DONT_SCHEDULE);
	}

	float outskip = p[0];
	float inskip = p[1];
	float dur = p[2];
	_amp = p[3];
	_inputchannel = (int) p[4];

	if (_inputchannel >= inputChannels()) {
		die("PVOC", "Requesting channel %d of a %d-channel input file",
			_inputchannel, inputChannels());
		return(DONT_SCHEDULE);
	}

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE; // no input
	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	
	_inputFrames = inputNSamps();
	_currentInputFrame = 0;

	// pick up arguments from command line

	// Note:  all these are class vars
	
	R	 = (int)SR;			/* sampling rate */
	_fftLen	 = (int)p[5];		/* FFT length */
	_windowLen	= (int)p[6];		/* window size */
	_decimation	 = (int)p[7];		/* decimation factor */
	_interpolation	 = (int)p[8];		/* interpolation factor */
	P	 = p[9];			/* oscillator bank pitch factor */
	Np	= (int)p[10];		/* linear prediction order */
	_oscThreshold  = (float)p[11];		/* synthesis threshhold */

#ifdef debug
	printf("initial PVOC parameters:\n" );
	printf("R (samprate) = %d\n", R );
	printf("N (fft len) = %d\n", _fftLen );
	printf("Nw (windowlen) = %d\n", _windowLen );
	printf("D (decimation) = %d\n", _decimation );
	printf("I (interpolation) = %d\n", I );
	printf("P = %g\n", P );
	printf("Np = %d\n", Np );
	printf("thresh = %g\n", _oscThreshold );
#endif
	if (_decimation <= 0) {
		die("PVOC", "decimation must be >= 1");
		return(DONT_SCHEDULE);
	}
	if (_interpolation <= 0) {
		die("PVOC", "interpolation must be >= 1");
		return(DONT_SCHEDULE);
	}
	if (_interpolation > _windowLen) {
		die("PVOC", "Window size must be >= interpolation factor");
		return(DONT_SCHEDULE);
	}
	TWOPI = 8.*atan(1.);
	obank = P != 0.;
	N2 = _fftLen>>1;
	Nw2 = _windowLen>>1;

    if (obank) {
        rtcmix_advise("PVOC", "Using oscillator bank resynthesis");
        initOscbank(N2, Np, R, _windowLen, _interpolation, P);
    }
	
	// Factors for convert() and unconvert()
	
	_fundamental = (float) R / (N2 * 2);
	_convertFactor = R / (_decimation * TWOPI);
	_unconvertFactor = TWOPI * _interpolation / R;
	_convertPhase = ::NewArray(N2 + 1);
	_unconvertPhase = ::NewArray(N2 + 1);

    rtcmix_advise("PVOC", "Running in %s mode, scaling factor %.3f, window len %d", obank ? "oscillator" : "IFFT", (float)_windowLen/_interpolation, _windowLen);
	// All buffer allocation done in configure()
	
/*
 * initialize input and output time values (in samples)
 */
	_in = -_windowLen;
	if ( _decimation )
		_on = (_in*_interpolation)/_decimation;
	else
		_on = _in;

#ifdef debug
	printf("_in: %d  _on: %d\n", _in, _on);
#endif
    
	// Get pv filter if present
	::GetFilter(&_pvFilter);
    RefCounted::ref(_pvFilter);

	return nSamps();
}

int PVOC::configure()
{
	/*
	 * allocate memory
	 */
	Wanal = ::NewArray(_windowLen);		/* analysis window */
	Wsyn = ::NewArray(_windowLen);		/* synthesis window */
	_pvInput = ::NewArray(_windowLen);	/* input buffer */
	Hwin = ::NewArray(_windowLen);		/* plain Hamming window */
	winput = ::NewArray(_windowLen);		/* windowed input buffer */
	lpcoef = ::NewArray(Np+1);	/* lp coefficients */
	_fftBuf = ::NewArray(_fftLen);		/* FFT buffer */
	channel = ::NewArray(_fftLen+2);	/* analysis channels */
	_pvOutput = ::NewArray(_windowLen);	/* output buffer */
	/*
	 * create windows
	 */
	makewindows( Hwin, Wanal, Wsyn, _windowLen, _fftLen, _interpolation, obank );

	// The input buffer is larger than BUFSAMPS so it can be filled with
	// enough samples (via multiple calls to rtgetin()) to satisfy the input.
	
	_inbuf = new BUFTYPE[inputChannels() * _windowLen];

	// The output buffer is also larger in order to allow at least a full
	// window of synthesized output to be stored.
	
	_outbuf = new BUFTYPE[_windowLen];	// XXX CHECK THIS SIZE
		
	return 0;
}

static const int L = 8192;		// also used in initOscBank

int PVOC::doUpdate()
{
#ifdef debug
	printf("doUpdate: %d/%d = %f%%\n", _currentInputFrame, _inputFrames, (float)_currentInputFrame*100.0/_inputFrames);
#endif
	_amp = update(3, _inputFrames, _currentInputFrame);
	/* decimation factor */
	double newDecimation = update(7, _inputFrames, _currentInputFrame);
	if ((int)newDecimation != _decimation && newDecimation >= 1.0) {
		_decimation = (int)newDecimation;
		_convertFactor = R / (_decimation * TWOPI);
#ifdef debug
		printf("_decimation updated to %d\n", _decimation);
#endif
	}
#ifdef ALLOW_DYNAMIC_INTERPOLATION
	/* interpolation factor */
	double newInterpolation = update(8, _inputFrames, _currentInputFrame);
	if ((int)newInterpolation != _interpolation && newInterpolation >= 1.0) {
		_interpolation	= (int)newInterpolation;
		if (_interpolation > _windowLen) {
			rtcmix_warn("PVOC", "interpolation factor limited to window size (%d)", Nw);
			_interpolation = _windowLen;
		}
		_unconvertFactor = TWOPI * _interpolation / R;
		_Iinv = 1./_interpolation;
	}
#endif
	/* oscillator bank pitch factor */
	double newPitch = update(9, _inputFrames, _currentInputFrame);
	if (newPitch != P) {
		const int N2 = _fftLen >> 1;
		P = newPitch;
		_Pinc = P*L/R;
		_ffac = P*PI/N2;
		if ( P > 1. )
			_NP = int(N2/P);
		else
			_NP = int(N2);
	}
	_oscThreshold  = (float)update(11, _inputFrames, _currentInputFrame);		/* synthesis threshhold */
	
	return 0;
}

int PVOC::run()
{
#ifdef debug
	printf("PVOC::run\n\n");
#endif

	// This runs the engine forward until the first buffer of output data is ready (see PVOC::shiftout()).
	// This compensates for the group delay of the windowed output.
	
	doUpdate();

	int outFramesNeeded = framesToRun();

	if (_cachedOutFrames)
	{
		int toCopy = min(_cachedOutFrames, outFramesNeeded);
		if (toCopy >= 0)
		{
#ifdef debug
			printf("\twriting %d of %d leftover frames from _outbuf at offset %d to rtbaddout\n",
				   toCopy, _cachedOutFrames, _outReadOffset);
#endif
			rtbaddout(&_outbuf[_outReadOffset], toCopy);
			increment(toCopy);
			_outReadOffset += toCopy;
			assert(_outReadOffset <= _outWriteOffset);
			if (_outReadOffset == _outWriteOffset)
				_outReadOffset = _outWriteOffset = 0;
#ifdef debug
			printf("\t_outbuf read offset %d, write offset %d\n",
				   _outReadOffset, _outWriteOffset);
#endif
			outFramesNeeded -= toCopy;
			_cachedOutFrames -= toCopy;
		}
	}

	while (outFramesNeeded > 0)
	{
#ifdef debug
		printf("\ttop of loop: needed=%d _in=%d _on=%d _windowLen=%d\n",
			   outFramesNeeded, _in, _on, _windowLen);
#endif	
		/*
		* analysis: input _decimation samples; window, fold and rotate input
		* samples into FFT buffer; take FFT; and convert to
		* amplitude-frequency (phase vocoder) form
		*/
		shiftin( _pvInput, _windowLen, _decimation);
		/*
		 * increment times
		 */
		_in += _decimation;
		_on += _interpolation;

		if ( Np ) {
			::vvmult( winput, Hwin, _pvInput, _windowLen );
			lpcoef[0] = ::lpa( winput, _windowLen, lpcoef, Np );
		/*			printf("%.3g/", lpcoef[0] ); */
		}
		::fold( _pvInput, Wanal, _windowLen, _fftBuf, _fftLen, _in );
		::rfft( _fftBuf, N2, FORWARD );
		convert( _fftBuf, channel, N2, _decimation, R );

	// 	if ( _interpolation == 0 ) {
	// 		if ( Np )
	// 			fwrite( lpcoef, sizeof(float), Np+1, stdout );
	// 		fwrite( channel, sizeof(float), N+2, stdout );
	// 		fflush( stdout );
	// /*		printf("\n" );
	// 		continue;
	// 	}
	/*
	 * at this point channel[2*i] contains amplitude data and
	 * channel[2*i+1] contains frequency data (in Hz) for phase
	 * vocoder channels i = 0, 1, ... N/2; the center frequency
	 * associated with each channel is i*f, where f is the
	 * fundamental frequency of analysis R/N; any desired spectral
	 * modifications can be made at this point: pitch modifications
	 * are generally well suited to oscillator bank resynthesis,
	 * while time modifications are generally well (and more
	 * efficiently) suited to overlap-add resynthesis
	 */

		if (_pvFilter) {
			_pvFilter->run(channel, N2);
		}

		if ( obank ) {
			/*
			 * oscillator bank resynthesis
			 */
			oscbank( channel, N2, lpcoef, Np, R, _windowLen, _interpolation, P, _pvOutput );
#if defined(debug) && 0
			printf("osc output (first 16):\n");
			for (int x=0;x<16;++x) printf("%g ",_pvOutput[x]);
			printf("\n");
#endif
			shiftout( _pvOutput, _windowLen, _interpolation, _on+_windowLen-_interpolation);
		}
		else {
			/*
			 * overlap-add resynthesis
			 */
			unconvert( channel, _fftBuf, N2, _interpolation, R );
			::rfft( _fftBuf, N2, INVERSE );
			::overlapadd( _fftBuf, _fftLen, Wsyn, _pvOutput, _windowLen, _on );
			// _interpolation samples written into _outbuf
			shiftout( _pvOutput, _windowLen, _interpolation, _on);
		}
	   // Handle case where last synthesized block extended beyond outFramesNeeded

		int framesToOutput = ::min(outFramesNeeded, _interpolation);
#ifdef debug
		printf("\tbottom of loop. framesToOutput: %d\n", framesToOutput);
#endif
		int framesAvailable = _outWriteOffset - _outReadOffset;
		framesToOutput = ::min(framesToOutput, framesAvailable);
		if (framesToOutput > 0) {
#ifdef debug
			printf("\twriting %d frames from offset %d to rtbaddout\n", 
				   framesToOutput, _outReadOffset);
#endif
			rtbaddout(&_outbuf[_outReadOffset], framesToOutput);
			increment(framesToOutput);
			_outReadOffset += framesToOutput;
			if (_outReadOffset == _outWriteOffset)
				_outReadOffset = _outWriteOffset = 0;
		}

		_cachedOutFrames = _outWriteOffset - _outReadOffset;
#ifdef debug
		if (_cachedOutFrames > 0) {
			printf("\tsaving %d samples left over\n", _cachedOutFrames);
		}
		printf("\toutbuf read offset %d, write offset %d\n\n",
			   _outReadOffset, _outWriteOffset);
#endif
		outFramesNeeded -= framesToOutput;
	}	/* while (outFramesNeeded > 0) */

	return framesToRun();
}

/*
 * shift next D samples into righthand end of array A of
 * length winLen, padding with zeros after last sample (A is
 * assumed to be initially 0);
 */
 
int PVOC::shiftin( float A[], int winLen, int D)
{
	int i, n;
	const int inchans = inputChannels();
	const float amp = _amp;
	
	if ( _valid < 0 )		/* first time only */
		_valid = winLen;
	// Shift what we already have over to beginning of buffer
#ifdef debug
	printf("\tshiftin: moving A[%d - %d] to A[%d - %d]\n",
		   D, winLen - 1, 0, winLen - D - 1);
#endif
	for ( i = 0 ; i < winLen - D ; ++i )
		A[i] = A[i+D];

	int framesToRead = D;
	int framecount = 0;

	while (framesToRead)
	{
		const int toCopy = min(_cachedInFrames, framesToRead);
#ifdef debug
		if (toCopy > 0)
			printf("\tshiftin: copying %d sampframes from input offset %d to A[%d]\n", 
			   	   toCopy, _inReadOffset, i);
#endif
		float *in = &_inbuf[_inReadOffset * inchans + _inputchannel];
		for (framecount = 0; i < winLen && framecount < toCopy; ++i, ++framecount) {  
			A[i] = *in * amp;
			in += inchans;
		}
		framesToRead -= framecount;
		_cachedInFrames -= framecount;
		_inReadOffset += framecount;
		if (_inReadOffset == _inWriteOffset)
			_inReadOffset = _inWriteOffset = 0;
		while (_cachedInFrames < framesToRead)
		{
			int toRead = min(framesToRead, RTBUFSAMPS/inchans);
#ifdef debug
			printf("\tshiftin: calling rtgetin for %d sampframes into offset %d\n",
				  toRead, _inWriteOffset);
#endif
			in = &_inbuf[_inWriteOffset * inchans];
			int framesRead = rtgetin(in, this, toRead*inchans) / inchans;
			_cachedInFrames += framesRead;
			_inWriteOffset += framesRead;
			_currentInputFrame += framesRead;
		}
	}
#ifdef debug
	if (_cachedInFrames)
		printf("\tshiftin: %d sampframes cached at offset %d\n",
	     	  _cachedInFrames, _inReadOffset);
#endif
	assert(_inWriteOffset < winLen);
	assert(_inReadOffset <= _inWriteOffset);

	_valid = D;
	
// 	if ( _valid < winLen ) {		/* pad with zeros after EOF */
// 		for ( i = _valid ; i < winLen ; ++i )
// 			A[i] = 0.;
// 		_valid -= D;
// 	}
	return 0;
}

/*
 * if output time n >= 0, output first I samples in
 * array A of length winLen, then shift A left by I samples,
 * padding with zeros after last sample
 */
void PVOC::shiftout( float A[], int winLen, int I, int n)
{
	int i;
	float *output = &_outbuf[_outWriteOffset];
#ifdef debug
	printf("\tshiftout: winLen=%d I=%d n=%d\n", winLen, I, n);
#endif
	if (n >= 0) {
#ifdef debug
		printf("\tshiftout: copying A[%d - %d] to _outbuf at write offset %d\n",
			   0, I-1, _outWriteOffset);
#endif
		for (i = 0; i < I; ++i) {
			output[i] = A[i];
		}
		_outWriteOffset += I;
#ifdef debug
		printf("\tshiftout: write offset incremented to %d\n", _outWriteOffset);
#endif
	}
#ifdef debug
	if (winLen > I) {
		printf("\tshiftout: moving A[%d - %d] to A[%d - %d]\n",
				I, I+winLen-I-1, 0, winLen-I-1);
	}
#endif
	for ( i = 0 ; i < winLen - I ; ++i ) {
		A[i] = A[i+I];
	}
#ifdef debug
	if (i < winLen) {
		printf("\tshiftout: zeroing A[%d - %d]\n", i, winLen - 1);
	}
#endif
	for ( ; i < winLen ; ++i )
		A[i] = 0.;
}

/*
 * S is a spectrum in rfft format, i.e., it contains N real values
 * arranged as real followed by imaginary values, except for first
 * two values, which are real parts of 0 and Nyquist frequencies;
 * convert first changes these into N/2+1 PAIRS of magnitude and
 * phase values to be stored in output array C; the phases are then
 * unwrapped and successive phase differences are used to compute
 * estimates of the instantaneous frequencies for each phase vocoder
 * analysis channel; decimation rate D and sampling rate R are used
 * to render these frequency values directly in Hz.
 */
void 
PVOC::convert(float S[], float C[], int N2, int D, int R)
{
	// Local copies
	register float *lastphase = _convertPhase;
	const float fundamental = _fundamental;
	const float factor = _convertFactor;
	
	/*
	 * unravel rfft-format spectrum: note that N2+1 pairs of
	 * values are produced
	 */
	for (int i = 0 ; i < N2 ; ++i ) {
		int real, imag, amp, freq;
		imag = freq = ( real = amp = i<<1 ) + 1;
		float a = S[real];
		float b = (i == 0) ? 0. : S[imag];
		/*
		 * compute magnitude value from real and imaginary parts
		 */
		C[amp] = hypotf( a, b );
		/*
		 * compute phase value from real and imaginary parts and take
		 * difference between this and previous value for each channel
		 */
		float phase, phasediff = 0.0f;

		if ( C[amp] != 0. ) {
			phasediff = ( phase = -atan2f( b, a ) ) - lastphase[i];
			lastphase[i] = phase;
			/*
			 * unwrap phase differences
			 */
			while ( phasediff > PI )
				phasediff -= TWOPI;
			while ( phasediff < -PI )
				phasediff += TWOPI;
		}
		/*
		 * convert each phase difference to Hz
		 */
		C[freq] = (phasediff * factor) + (i * fundamental);
	}
	// i == N2 case
	{
		int real, imag, amp, freq;
		imag = freq = ( real = amp = N2<<1 ) + 1;
		float a = S[1];
		float b = 0.;
		C[amp] = hypotf( a, b );
		float phase, phasediff = 0.0f;

		if ( C[amp] != 0. ) {
			phasediff = ( phase = -atan2f( b, a ) ) - lastphase[N2];
			lastphase[N2] = phase;
			while ( phasediff > PI )
				phasediff -= TWOPI;
			while ( phasediff < -PI )
				phasediff += TWOPI;
		}
		C[freq] = (phasediff * factor) + (N2 * fundamental);
	}
}

/*
 * unconvert essentially undoes what convert does, i.e., it
 * turns N2+1 PAIRS of amplitude and frequency values in
 * C into N2 PAIR of complex spectrum data (in rfft format)
 * in output array S; sampling rate R and interpolation factor
 * I are used to recompute phase values from frequencies
 */
void 
PVOC::unconvert( float C[], float S[], int N2, int I, int R )
{
	// Local copies
	register float *lastphase = _unconvertPhase;
	const float fundamental = _fundamental;
	const float factor = _unconvertFactor;

	/*
	 * subtract out frequencies associated with each channel,
	 * compute phases in terms of radians per I samples, and
	 * convert to complex form
	 */
	for (int i = 0 ; i < N2 ; ++i) {
		int real, imag, amp, freq;
		imag = freq = ( real = amp = i<<1 ) + 1;
		float mag = C[amp];
		lastphase[i] += C[freq] - i*fundamental;
		float phase = lastphase[i]*factor;
		S[real] = mag*cosf( phase );
		S[imag] = -mag*sinf( phase );
	}
	// i == N2 case
	{
		int real = 1, imag, amp, freq;
		imag = freq = ( amp = N2<<1 ) + 1;
		float mag = C[amp];
		lastphase[N2] += C[freq] - N2 * fundamental;
		float phase = lastphase[N2]*factor;
		S[real] = mag * cosf( phase );
	}
}

void
PVOC::initOscbank(int N, int npoles, int R, int Nw, int I, float P)
{
	_lastAmp = ::NewArray(N+1);
	_lastFreq = ::NewArray(N+1);
	_index = ::NewArray(N+1);
	_table = ::NewArray(L);

	const float tabscale = npoles ? 2./Nw : ( Nw >= N ? N : 8*N );
	const float TWOPIoL = TWOPI/L;
	for (int n = 0; n < L; ++n)
		_table[n] = tabscale*cosf( TWOPIoL*n );
	_Iinv = 1./_interpolation;
	_Pinc = P*L/R;
	_ffac = P*PI/N;
	if ( P > 1. )
		_NP = int(N/P);
	else
		_NP = int(N);
}

/*
 * oscillator bank resynthesizer for phase vocoder analyzer
 * uses sum of N+1 cosinusoidal table lookup oscillators to 
 * compute I (interpolation factor) samples of output O
 * from N+1 amplitude and frequency value-pairs in C;
 * frequencies are scaled by P
 */
void
PVOC::oscbank(float C[], int N, float lpcoef[], int npoles,
			  int R, int Nw, int I, float P, float O[])
{
	// Local copies
	const float thresh = _oscThreshold;
	const float Pinc = _Pinc, ffac = _ffac, Iinv = _Iinv;
	const int NP = _NP;
	float *lastfreq = _lastFreq;
	float *lastamp = _lastAmp;
	float *index = _index;
	float *table = _table;
	
#ifdef debug
	printf("\toscbank: N=%d Nw=%d I=%d P=%g\n", N, Nw, I, P);
#endif
	/*
	 * for each channel, compute I samples using linear
	 * interpolation on the amplitude and frequency
	 * control values
	 */
	for (int chan = npoles ? (int)P : 0; chan < NP; ++chan) {
		const int amp = ( chan << 1 );
		const int freq = amp + 1;
		// If this bin's amp is less than threshold, ramp to zero if prev
		//	bin was active.
		if (C[amp] < thresh) {
			if (lastamp[chan] >= thresh) {
				C[amp] = 0.0f;	// This will generate a fade-out on this chan
			}
			else {
				lastamp[chan] = 0.0f;	// Already faded out. Skip it.
				continue;
			}
		}
		C[freq] *= Pinc;

		register float a, f;
		register const float finc = ( C[freq] - ( f = lastfreq[chan] ) ) * Iinv;
	/*
	 * if linear prediction specified, REPLACE phase vocoder amplitude
	 * measurements with linear prediction estimates
	 */
		if ( npoles ) {
			if ( f == 0. )
				C[amp] = 0.;
			else
				C[amp] = ::lpamp( chan*ffac, lpcoef[0], lpcoef, npoles );
		}
		register const float ainc = ( C[amp] - ( a = lastamp[chan] ) ) * Iinv;
		register float address = index[chan];
	/*
	 * accumulate the I samples from each oscillator into
	 * output array O (initially assumed to be zero);
	 * f is frequency in Hz scaled by oscillator increment
	 * factor and pitch (Pinc); a is amplitude;
	 */
		for (int n = 0 ; n < I ; ++n) {
			O[n] += a*table[ (int) address ] ;  
			address += f;
			while ( address >= L )
				address -= L;
			while ( address < 0 )
				address += L;
			a += ainc;
			f += finc;
		} 
	/*
	 * save current values for next iteration
	 */
		lastfreq[chan] = C[freq];
		lastamp[chan] = C[amp];
		index[chan] = address;
	}
}

Instrument *makePVOC()
{
   PVOC *inst;

   inst = new PVOC();
   inst->set_bus_config("PVOC");

   return inst;
}

#ifndef EMBEDDED
/* The rtprofile introduces the instruments to the RTcmix core, and
   associates a Minc name (in quotes below) with the instrument. This
   is the name the instrument goes by in a Minc script.
*/
void rtprofile()
{
   RT_INTRO("PVOC", makePVOC);
}
#endif

