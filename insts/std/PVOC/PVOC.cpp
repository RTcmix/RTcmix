 #include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <mixerr.h>
#include <rt.h>
#include <rtdefs.h>
#include <string.h>

#include "pv.h"
#include "PVOC.h"

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
*  p3 = amp
*  p4 = input channel
*  p5 = fft size
*  p6 = window size
*  p7 = decimation amount (readin)
*  p8 = interpolation amount (putout)
*  p9 = pitch multiplier
*  p10 = npoles
*
*/

PVOC::PVOC() : _first(1), _valid(-1), _currentsample(0)
{
	obank = 0;
	_inOffset = _outOffset = 0;
	_sampsRead = 0;
	_leftOver = 0;
	_inbuf = NULL;
	_outbuf = NULL;
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
	}
	memset(arr, 0, size * sizeof(float));
	return arr;
}

int PVOC::init(float *p, int n_args)
{
	if (!n_args || n_args < 9) {
		die("LPCIN",
		"p[0]=outskip, p[1]=inskip, p[2]=duration, p[3]=amp, p[4]=input_chan, p[5]=fft_size, p[6]=window_size, p[7]=decim, p[8]=interp, [ p[9]=pitch_mult ]\n");
	}
	float outskip = p[0];
	float inskip = p[1];
	float dur = p[2];
	_amp = p[3];
	_inputchannel = (int) p[4];

	rtsetinput(inskip, this);
	rtsetoutput(outskip, dur, this);
	

	// pick up arguments from command line

	// Note:  all these are class vars
	
	R	 = (int)SR;			/* sampling rate */
	N	 = (int)p[5];		/* FFT length */
	Nw	= (int)p[6];		/* window size */
	D	 = (int)p[7];		/* decimation factor */
	I	 = (int)p[8];		/* interpolation factor */
	P	 = p[9];			/* oscillator bank pitch factor */
	Np	= (int)p[10];		/* linear prediction order */
	_oscThreshold  = (int)p[11];		/* synthesis threshhold */

/*	freopen( "pv.out", "w", stderr ); */
#ifdef debug
	printf("pv parameters:\n" );
	printf("R = %d\n", R );
	printf("N = %d\n", N );
	printf("Nw = %d\n", Nw );
	printf("D = %d\n", D );
	printf("I = %d\n", I );
	printf("P = %g\n", P );
	printf("Np = %d\n", Np );
	printf("thresh = %g\n", _oscThreshold );
#endif

	if (I > Nw) {
		die("PVOC", "Window size must be >= decimation factor");
	}
	TWOPI = 8.*atan(1.);
	obank = P != 0.;
	N2 = N>>1;
	Nw2 = Nw>>1;

	if (obank)
		initOscbank(N2, Np, R, Nw, I, P);
	
	// Factors for convert() and unconvert()
	
	_fundamental = (float) R / (N2 * 2);
	_convertFactor = R / (D * TWOPI);
	_unconvertFactor = TWOPI * I / R;
	_convertPhase = ::NewArray(N2 + 1);
	_unconvertPhase = ::NewArray(N2 + 1);
	
/*
 * allocate memory
 */
	Wanal = ::NewArray(Nw);		/* analysis window */
	Wsyn = ::NewArray(Nw);		/* synthesis window */
	_pvInput = ::NewArray(Nw);	/* input buffer */
	Hwin = ::NewArray(Nw);		/* plain Hamming window */
	winput = ::NewArray(Nw);		/* windowed input buffer */
	lpcoef = ::NewArray(Np+1);	/* lp coefficients */
	_fftBuf = ::NewArray(N);		/* FFT buffer */
	channel = ::NewArray(N+2);	/* analysis channels */
	_pvOutput = ::NewArray(Nw);	/* output buffer */
/*
 * create windows
 */
	makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank );
/*
 * initialize input and output time values (in samples)
 */
	_in = -Nw;
	if ( D )
		_on = (_in*I)/D;
	else
		_on = _in;

	return NSamps();
}

int PVOC::run()
{
	// If this is first call to run, allocate input buffer, which
	//	must persist across calls to run.
	if (_inbuf == NULL)
		_inbuf = new BUFTYPE[InputChannels() * RTBUFSAMPS];

	if (_outbuf == NULL)
		_outbuf = new BUFTYPE[Nw];	// XXX CHECK THIS SIZE
		
	/* You MUST call the base class's run method here. */
	Instrument::run();
#ifdef debug
	printf("PVOC::run\n\n");
#endif
	int outFramesNeeded = FramesToRun();
	
	if (_leftOver)
	{
		int toCopy = min(_leftOver, outFramesNeeded);
		if (_on > 0)
		{
#ifdef debug
			printf("\twriting %d of %d leftover frames at offset %d to outbus\n", 
				   toCopy, _leftOver, _outOffset);
#endif
			rtbaddout(&_outbuf[_outOffset], toCopy);
		}
		_outOffset += toCopy;
		if (_outOffset >= _leftOver)
			_outOffset = 0;
		outFramesNeeded -= toCopy;
		_leftOver -= toCopy;
	}
	
	while (outFramesNeeded > 0)
	{
#ifdef debug
		printf("\ttop of loop: needed=%d _in=%d _on=%d Nw=%d\n",
			   outFramesNeeded, _in, _on, Nw);
#endif	
		int inputFramesNeeded = D;
		
		while (inputFramesNeeded)
		{
			int toRead = min(RTBUFSAMPS, D);
#ifdef debug
			printf("\tcalling rtgetin for %d frames\n", toRead);
#endif
//			_sampsRead = rtgetin(&_inbuf[_inOffset], this, toRead * InputChannels());
			_sampsRead = rtgetin(&_inbuf[0], this, toRead * InputChannels());
//			_inOffset += _sampsRead;
			inputFramesNeeded -= _sampsRead;
		}
		/*
		* analysis: input D samples; window, fold and rotate input
		* samples into FFT buffer; take FFT; and convert to
		* amplitude-frequency (phase vocoder) form
		*/
		shiftin( _pvInput, Nw, D, &_inbuf[0]);
		/*
		 * increment times
		 */
		_in += D;
		_on += I;

		if ( Np ) {
			::vvmult( winput, Hwin, _pvInput, Nw );
			lpcoef[0] = ::lpa( winput, Nw, lpcoef, Np );
		/*			printf("%.3g/", lpcoef[0] ); */
		}
		::fold( _pvInput, Wanal, Nw, _fftBuf, N, _in );
		::rfft( _fftBuf, N2, FORWARD );
		convert( _fftBuf, channel, N2, D, R );

	// 	if ( I == 0 ) {
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
		if ( obank ) {
			/*
			 * oscillator bank resynthesis
			 */
			oscbank( channel, N2, lpcoef, Np, R, Nw, I, P, _pvOutput );
#ifdef debug
			printf("osc output (first 16):\n");
			for (int x=0;x<16;++x) printf("%g ",_pvOutput[x]);
			printf("\n");
#endif
			shiftout( _pvOutput, Nw, I, _on+Nw-I, &_outbuf[0]);
		}
		else {
			/*
			 * overlap-add resynthesis
			 */
			unconvert( channel, _fftBuf, N2, I, R );
			::rfft( _fftBuf, N2, INVERSE );
			::overlapadd( _fftBuf, N, Wsyn, _pvOutput, Nw, _on );
			// I samples written into _outbuf
			shiftout( _pvOutput, Nw, I, _on, &_outbuf[0]);
		}
 	   // Handle case where last synthesized block extended beyond outFramesNeeded

		int framesToOutput = ::min(outFramesNeeded, I);
#ifdef debug
		printf("bottom of loop. framesToOutput: %d cursamp = %d\n",
			   framesToOutput, cursamp);
#endif
		if (_on >= 0) {
#ifdef debug
			printf("\twriting %d leftover frames at offset %d to outbus\n", 
				   framesToOutput, _outOffset);
#endif
			rtbaddout(&_outbuf[_outOffset], framesToOutput);
		}
		_leftOver = I - framesToOutput;
		if (_leftOver > 0) {
			_outOffset += framesToOutput;
#ifdef debug
	   	 	printf("saving %d samples left over at offset %d\n",
				   _leftOver, _outOffset);
#endif
		}
		outFramesNeeded -= framesToOutput;
	}

	return FramesToRun();
}

/*
 * shift next D samples into righthand end of array A of
 * length winLen, padding with zeros after last sample (A is
 * assumed to be initially 0);
 */
 
int PVOC::shiftin( float A[], int winLen, int D, float *input)
{
	int i;
	int nsamps = NSamps();
	int inchans = InputChannels();
	if ( _valid < 0 )		/* first time only */
		_valid = winLen;
	// Shift what we already have over to beginning of buffer
#ifdef debug
	printf("\tshiftin: moving A[%d - %d] to A[%d - %d]\n",
		   D, D + winLen - D - 1, 0, winLen - D - 1);
#endif
	for ( i = 0 ; i < winLen - D ; ++i )
		A[i] = A[i+D];
	_currentsample += D;
	int framesToRead = D;
	float *in = input;
#ifdef debug
	printf("\tshiftin: copying %d samples from input to A[%d]\n", D, winLen - D);
#endif
	for ( i = winLen - D ; i < winLen ; ++i ) {  
		A[i] = *in;
		in += inchans;
	}
	_valid = framesToRead;
	
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
void PVOC::shiftout( float A[], int winLen, int I, int n, float *output)
{
	int i;
#ifdef debug
	printf("\tshiftout: winLen=%d I=%d n=%d\n", winLen, I, n);
#endif
//	 if ( n >= 0 ) {
// 		printf("\tshiftout: rtbaddout writing %d frames\n", I);
//		 rtbaddout( A, I);
// 		increment(I);
// 	}
	if ( n >= 0 ) {
#ifdef debug
		printf("\tshiftout: copying A[%d - %d] to final output buffer\n", 0, I-1);
#endif
		for (i = 0; i < I; ++i)
			output[i] = A[i];
		increment(I);
	}
#ifdef debug
	if (winLen > I) {
		printf("\tshiftout: moving A[%d - %d] to A[%d - %d]\n",
				I, I+winLen-I-1, 0, winLen-I-1);
	}
#endif
	for ( i = 0 ; i < winLen - I ; ++i )
		A[i] = A[i+I];
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

static const int L = 8192;
static const unsigned int Lmask = L - 1;

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
	_Iinv = 1./I;
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
	printf("oscbank: N=%d Nw=%d I=%d P=%g\n", N, Nw, I, P);
#endif
	/*
	 * for each channel, compute I samples using linear
	 * interpolation on the amplitude and frequency
	 * control values
	 */
	for (int chan = npoles ? (int)P : 0; chan < NP; ++chan) {
		int amp;
		int freq = ( amp = ( chan << 1 ) ) + 1;
		if (C[amp] < thresh) /* skip the little ones */
			continue;
		C[freq] *= Pinc;

		register float a, f, finc;
		finc = ( C[freq] - ( f = lastfreq[chan] ) ) * Iinv;
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
		register float ainc = ( C[amp] - ( a = lastamp[chan] ) ) * Iinv;
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

/* The rtprofile introduces the instruments to the RTcmix core, and
   associates a Minc name (in quotes below) with the instrument. This
   is the name the instrument goes by in a Minc script.
*/
void rtprofile()
{
   RT_INTRO("PVOC", makePVOC);
}
