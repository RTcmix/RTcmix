// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

// Base class for SPECTACLE2* instruments
// John Gibson <johgibso at indiana dot edu>, 6/9/05.

//#define DUMP
//#define DEBUG
//#define DEBUG_BINGROUPS
//#define CHECK_BINGROUPS

#include "SPECTACLE2_BASE.h"
#include <float.h>
#include <Option.h>

#ifndef cosf
	#define cosf(x) cos((x))
#endif
#ifndef sinf
	#define sinf(x) sin((x))
#endif
#ifndef hypotf
	#define hypotf(x, y) hypot((x), (y))
#endif
#ifndef atan2f
	#define atan2f(y, x) atan2((y), (x))
#endif

const int kMaxWindowLen = kMaxFFTLen * 8;
const int kMinOverlap = 1;
const int kMaxOverlap = 64;


// ---------------------------------------------------------- SPECTACLE2_BASE --
SPECTACLE2_BASE::SPECTACLE2_BASE() :
	_nyquist(SR / 2),
	_anal_bins(NULL),
	_control_table_size(0),
	_branch(0),
	_prev_bg_ignorevals(-1),
	_iamp_pfield_index(0),       // mark as invalid
	_window_pfield_index(0),
	_wet(-FLT_MAX),
	_ringdur(0.0f),
	_iamp(1.0f),     // in case no input amp envelope
	_minfreq(-FLT_MAX), _rawmaxfreq(-FLT_MAX),
	_anal_window(NULL), _synth_window(NULL),
	_input(NULL), _output(NULL),
	_inbuf(NULL), _outbuf(NULL),
	_binmaptable(NULL),
	_fft(NULL),
	_bucket(NULL),
	_dry_delay(NULL)
{
}


// --------------------------------------------------------- ~SPECTACLE2_BASE --
SPECTACLE2_BASE::~SPECTACLE2_BASE()
{
	delete [] _inbuf;
	delete [] _outbuf;
	delete [] _input;
	delete [] _output;
	delete [] _anal_window;
	delete [] _synth_window;
	delete [] _anal_bins;
	delete [] _bin_groups;
	delete _bucket;
	delete _fft;
	delete _dry_delay;
}


// ------------------------------------------------------- resample_functable --
// Accepts a function table of <oldsize> elements, describing a series of
// line segments. Resamples this table using linear interpolation, so that
// its shape is described by <newsize> elements. Returns a new table,
// allocated with new, containing this resampled data.  Caller is responsible
// for deleting this table.

double *SPECTACLE2_BASE::resample_functable(const double *table,
	const int oldsize, const int newsize)
{
	double *newtable = new double [newsize];
	if (newtable == NULL)
		return NULL;

	if (newsize == oldsize) {						 // straight copy
		for (int i = 0; i < newsize; i++)
			newtable[i] = table[i];
	}
	else {
		const double incr = double(oldsize) / double(newsize);
		double f = 0.0;
		for (int i = 0; i < newsize; i++) {
			int n = int(f);
			double frac = f - double(n);
			double diff = 0.0;
			if (frac) {
				double next = (n + 1 < oldsize) ? table[n + 1] : table[oldsize - 1];
				diff = next - table[n];
			}
			newtable[i] = table[n] + (diff * frac);
			f += incr;
		}
	}
	return newtable;
}


// ------------------------------------------------------------- make_windows --
int SPECTACLE2_BASE::make_windows(const double *table, const int len)
{
	if (table) {
		double *tmptab = resample_functable(table, len, _window_len);
		if (tmptab == NULL)
			return -1;
		for (int i = 0; i < _window_len; i++)
			_anal_window[i] = _synth_window[i] = tmptab[i];
		delete [] tmptab;
	}
	else {	// make a Hamming window
		for (int i = 0; i < _window_len; i++)
			_anal_window[i] = _synth_window[i] = 0.54f - 0.46f
									 * cosf(2.0f * M_PI * i / (_window_len - 1));
	}

	// When _window_len > _fftlen, also apply interpolating (sinc) windows to
	// ensure that window is 0 at increments of _fftlen away from the center
	// of the analysis window and of decimation away from the center of the
	// synthesis window.

	if (_window_len > _fftlen) {
		float x = -(_window_len - 1) / 2.0;
		for (int i = 0; i < _window_len; i++, x += 1.0f)
			if (x != 0.0f) {
				_anal_window[i] *= _fftlen * sin(M_PI * x / _fftlen) / (M_PI * x);
				if (_decimation)
					_synth_window[i] *= _decimation * sin(M_PI * x / _decimation)
					                                               / (M_PI * x);
			}
	}

	// Normalize windows for unity gain across unmodified
	// analysis-synthesis procedure.

	float sum = 0.0f;
	for (int i = 0; i < _window_len; i++)
		sum += _anal_window[i];

	for (int i = 0; i < _window_len; i++) {
		float afac = 2.0f / sum;
		float sfac = _window_len > _fftlen ? 1.0f / afac : afac;
		_anal_window[i] *= afac;
		_synth_window[i] *= sfac;
	}

	if (_window_len <= _fftlen && _decimation) {
		sum = 0.0f;
		for (int i = 0; i < _window_len; i += _decimation)
			sum += _synth_window[i] * _synth_window[i];
		sum = 1.0f / sum;
		for (int i = 0; i < _window_len; i++)
			_synth_window[i] *= sum;
	}

	return 0;
}


// --------------------------------------------------------------------- init --
int SPECTACLE2_BASE::init(double p[], int n_args)
{
	_nargs = n_args;
#ifdef NOTYET
	_print_stats = Option::printStats();
#else
	_print_stats = true;
#endif

	if (getargs(p, n_args) < 0)	// subclass gets args
		return DONT_SCHEDULE;

	// NB: subclass init will have already grabbed all pfields except these...
	const float outskip = p[0];
	const float inskip = p[1];
	_inputdur = p[2];
	_oamp = p[3];

	// Make sure FFT length is a power of 2 <= kMaxFFTLen.
	bool valid = false;
	for (int x = 1; x <= kMaxFFTLen; x *= 2) {
		if (_fftlen == x) {
			valid = true;
			break;
		}
	}
	if (!valid)
		return die(instname(), "FFT length must be a power of two <= %d",
		                                                       kMaxFFTLen);

	_half_fftlen = _fftlen / 2;
	_fund_anal_freq = SR / float(_fftlen);

	// Make sure window length is a power of 2 >= FFT length.
	valid = false;
	for (int x = _fftlen; x <= kMaxWindowLen; x *= 2) {
		if (_window_len == x) {
			valid = true;
			break;
		}
	}
	if (!valid)
		return die(instname(),
		           "Window length must be a power of two >= FFT length (%d)\n"
		           "and <= %d.", _fftlen, kMaxWindowLen);

	// Make sure _overlap is a power of 2 in allowed range.
	valid = false;
	for (int x = kMinOverlap; x <= kMaxOverlap; x *= 2) {
		if (_overlap == x) {
			valid = true;
			break;
		}
	}
	if (!valid)
		return die(instname(),
		           "Overlap must be a power of two between %d and %d.",
		           kMinOverlap, kMaxOverlap);

	// derive decimation from overlap
	_decimation = int(_fftlen / _overlap);

	// create this now, because subinit will need it
	_bin_groups = new int [_half_fftlen + 1];

	// subclass init -- must follow FFT and bin groups init; sets _ringdur
	if (subinit(p, n_args) < 0)
		return DONT_SCHEDULE;

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;
	if (_inchan >= inputChannels())
		return die(instname(), "You asked for channel %d of a %d-channel input.",
		                       _inchan, inputChannels());

	// Latency is the delay before the FFT looks at actual input rather than
	// zero-padding.	Need to let inst run long enough to compensate for this,
	// as well as to span the user's requested ring-down duration.

	_window_len_minus_decimation = _window_len - _decimation;
	_latency = _window_len_minus_decimation;
	const float latency_dur = _latency / SR;
	if (rtsetoutput(outskip, latency_dur + _inputdur + _ringdur, this) == -1)
		return DONT_SCHEDULE;
	_input_frames = int(_inputdur * SR + 0.5);	// without latency_dur
	_input_end_frame = _input_frames + _latency;

	DPRINT2("_fftlen=%d, _decimation=%d\n", _fftlen, _decimation);
	DPRINT3("_latency=%d, _input_frames=%d, ring frames = %d\n",
		_latency, _input_frames, int(_ringdur * SR + 0.5));

	return nSamps();
}


// ---------------------------------------------------------------- configure --
int SPECTACLE2_BASE::configure()
{
	_inbuf = new float [RTBUFSAMPS * inputChannels()];
	_input = new float [_window_len];            // interior input buffer
	_output = new float [_window_len];           // interior output buffer
	_anal_bins = new float [_fftlen + 2];        // analysis bins
	if (_inbuf == NULL || _input == NULL || _output == NULL || _anal_bins == NULL)
		return -1;
	for (int i = 0; i < _window_len; i++)
		_input[i] = _output[i] = 0.0f;

	// Delay dry output by latency to sync with wet sig.
	_dry_delay = new Odelay(_window_len);
   _dry_delay->setdelay(_latency);

	// Read index chases write index by _decimation; add 2 extra locations to
	// keep read point from stepping on write point.  Verify with asserts in
	// increment_out_*_index().
	_outframes = _decimation + 2;
	_out_read_index = _outframes - _decimation;
	_out_write_index = 0;
	_outbuf = new float [_outframes];
	if (_outbuf == NULL)
		return -1;
	for (int i = 0; i < _outframes; i++)
		_outbuf[i] = 0.0f;
	DPRINT1("_outframes: %d\n", _outframes);

	_bucket = new Obucket(_decimation, process_wrapper, (void *) this);

	_fft = new Offt(_fftlen);
	_fft_buf = _fft->getbuf();

	_anal_window = new float [_window_len];
	_synth_window = new float [_window_len];
	if (_anal_window == NULL || _synth_window == NULL)
		return -1;
	int len;
	double *window = (double *) getPFieldTable(_window_pfield_index, &len);
	if (make_windows(window, len) != 0)
		return DONT_SCHEDULE;

	if (subconfigure() != 0)
		return -1;

	return 0;
}


// ------------------------------------------------------------ prepare_input --
// <buf> contains <_decimation> samples (mono) from most recent input.

void SPECTACLE2_BASE::prepare_input(const float buf[])
{
	DPRINT1("prepare_input (currentFrame()=%d)\n", currentFrame());

	// Shift samples in <_input> from right to left by <_decimation>,
	// leaving a hole of <_decimation> slots at right end.

	for (int i = 0; i < _window_len_minus_decimation; i++)
		_input[i] = _input[i + _decimation];

	// Copy <_decimation> samples from <buf> to right end of <_input>.

	for (int i = _window_len_minus_decimation, j = 0; i < _window_len; i++, j++)
		_input[i] = buf[j];

	// Multiply input array by analysis window, both of length <_window_len>.
	// Fold and rotate windowed real input into FFT buffer.

	for (int i = 0; i < _fftlen; i++)
		_fft_buf[i] = 0.0f;
	int j = currentFrame() % _fftlen;
	for (int i = 0; i < _window_len; i++) {
		_fft_buf[j] += _input[i] * _anal_window[i];
		if (++j == _fftlen)
			j = 0;
	}
}


// ------------------------------------------------------- cartesian_to_polar --
// <_fft_buf> is a spectrum containing <_half_fftlen> + 1 complex values,
// arranged in pairs of real and imaginary values, except for the first two
// values, which are the real parts of the DC and Nyquist frequencies.  Converts
// these into <_half_fftlen> + 1 pairs of magnitude and phase values, and
// stores them into the output array <_anal_bins>.

void SPECTACLE2_BASE::cartesian_to_polar()
{
	// handle DC and Nyquist first (imag is zero)
	_anal_bins[0] = _fft_buf[0];
	_anal_bins[1] = (_fft_buf[0] < 0.0f) ? M_PI : 0.0f;
	_anal_bins[_fftlen] = _fft_buf[1];
	_anal_bins[_fftlen + 1] = (_fft_buf[1] < 0.0f) ? M_PI : 0.0f;

	for (int i = 1; i < _half_fftlen; i++) {
		int mag_index = i << 1;
		int phase_index = mag_index + 1;
		float real = _fft_buf[mag_index];
		float imag = _fft_buf[phase_index];
		_anal_bins[mag_index] = hypotf(real, imag);
		_anal_bins[phase_index] = -atan2f(imag, real);
	}
}


// ------------------------------------------------------- polar_to_cartesian --
// Turns <_half_fftlen> + 1 pairs of magnitude and phase values in <_anal_bins>
// into pairs of complex spectrum data in <_fft_buf>, with the Nyquist real
// value replacing the DC imaginary value.

void SPECTACLE2_BASE::polar_to_cartesian()
{
	// handle DC and Nyquist first (real only)
	_fft_buf[0] = _anal_bins[0] * cosf(_anal_bins[1]);
	_fft_buf[1] = _anal_bins[_fftlen] * cosf(_anal_bins[_fftlen + 1]);

	for (int i = 1; i < _half_fftlen; i++) {
		int real_index = i << 1;
		int imag_index = real_index + 1;
		float mag = _anal_bins[real_index];
		float phase = _anal_bins[imag_index];
		_fft_buf[real_index] = mag * cosf(phase);
		_fft_buf[imag_index] = -mag * sinf(phase);
	}
}


// ----------------------------------------------------------- prepare_output --
void SPECTACLE2_BASE::prepare_output()
{
	// overlap-add <_fft_buf> real data into <_output>
	int j = currentFrame() % _fftlen;
	for (int i = 0; i < _window_len; i++) {
		_output[i] += _fft_buf[j] * _synth_window[i];
		if (++j == _fftlen)
			j = 0;
	}

	// transfer samples from <_output> to outer output buffer.
	for (int i = 0; i < _decimation; i++) {
		_outbuf[_out_write_index] = _output[i];
		increment_out_write_index();
	}

	// shift samples in <_output> from right to left by <_decimation>
	for (int i = 0; i < _window_len_minus_decimation; i++)
		_output[i] = _output[i + _decimation];
	for (int i = _window_len_minus_decimation; i < _window_len; i++)  
		_output[i] = 0.0f;
}


// ---------------------------------------------------------- process_wrapper --
// Called by Obucket whenever the input bucket is full (i.e., has _decimation
// samps).  This static member wrapper function lets us call a non-static member
// function, which we can't pass directly as a callback to Obucket.
// See http://www.newty.de/fpt/callback.html for one explanation of this.

void SPECTACLE2_BASE::process_wrapper(const float buf[], const int len,
                                                                  void *obj)
{
	SPECTACLE2_BASE *myself = (SPECTACLE2_BASE *) obj;
	myself->process(buf, len);
}


// ------------------------------------------------------------------ process --
void SPECTACLE2_BASE::process(const float *buf, const int)
{
	DPRINT("SPECTACLE2_BASE::process\n");

	const bool reading_input = (currentFrame() < _input_end_frame);
	if (reading_input) {
		DPRINT1("taking input...currentFrame()=%d\n", currentFrame());
		prepare_input(buf);
		_fft->r2c();
		cartesian_to_polar();
	}
	modify_analysis(reading_input);
	polar_to_cartesian();
	_fft->c2r();
	prepare_output();
}


// ------------------------------------------------------------ set_freqrange --
// Update the frequency range within which control tables operate.  Return true
// if range has actually changed, resulting in an update_bin_groups call.
// Otherwise, return false.

bool SPECTACLE2_BASE::set_freqrange(float min, float max, const char *type)
{
	if (min != _minfreq || max != _rawmaxfreq) {
		_rawmaxfreq = max;
		if (max == 0.0f)
			max = _nyquist;
		if (max < min)
			_fswap(&min, &max);
		if (max > _nyquist)
			max = _nyquist;
		_minfreq = _fclamp(0.0f, min, max);
		update_bin_groups(_bin_groups, _minfreq, max, _control_table_size, type);
		return true;
	}
	return false;
}


// -------------------------------------------------------- update_bin_groups --
// Update a bin groups array.  (See .h for a description of bin groups.)  Note 
// that this is called from set_freqrange and any subclass equivalents.

void SPECTACLE2_BASE::update_bin_groups(
	int bin_groups[],
	const float minfreq,
	const float maxfreq,
	const int control_table_size,
	const char *type)		// to identify bin groups to user
{
	if (control_table_size == 0)				// no control table
		return;

	const int nbins = _half_fftlen + 1;		// including DC and Nyquist

	// The optional <_binmaptable> has <control_table_size> elements, each giving
	// the number of adjacent FFT bins controlled by that element in any of the
	// control tables (e.g., EQ).  Copy this information into the <bin_groups>
	// array, which is sized to <nbins>.  We ignore minfreq and maxfreq.

	if (_binmaptable != NULL) {
		int bidx = 0;
		for (int i = 0; i < control_table_size; i++) {
			int bincount = int(_binmaptable[i]);
			while (bincount-- > 0) {
				if (bidx == nbins)
					goto highcount;
				bin_groups[bidx++] = i;
			}
		}
highcount:
		if (bidx < nbins) {
			const int last = control_table_size - 1;
			while (bidx < nbins)
				bin_groups[bidx++] = last;
		}
	}

	// If there is no binmap table...
	// If freq. range is full bandwidth, and control table size is >= to the
	// number of bins, then use linear mapping of control table slots to bins,
	// as in SPECTACLE v1.  If table is larger than the number of bins, warn 
	// about ignoring the extra table slots.

	else if (minfreq == 0 && maxfreq == _nyquist
	                  && control_table_size >= _half_fftlen) {
		for (int i = 0; i < _half_fftlen; i++)
			bin_groups[i] = i;

		// Let last table element control last two bins (incl. Nyquist), so that
		// we can ask user to size arrays to fftlen/2, rather than fftlen/2 + 1.
		bin_groups[_half_fftlen] = _half_fftlen - 1;
		if (control_table_size > _half_fftlen) {
			const int ignorevals = control_table_size - _half_fftlen;
			if (ignorevals != _prev_bg_ignorevals) {
				rtcmix_warn(instname(), "Control table of size %d too large for "
			                    "frequency range...ignoring last %d values",
			                    control_table_size, ignorevals);
				_prev_bg_ignorevals = ignorevals;
			}
		}
	}

	// Otherwise, there is either a freq. range that is not full bandwidth, or
	// the control table size is less than the number of bins.  If the former,
	// we insure that the first array slot affects all bins whose frequencies
	// are below (and possibly a little above) the min. freq., and that the last
	// array slot affects all bins whose frequencies are above (and possibly a
	// little below) the max. freq.  If the control table is smaller than the
	// number of bins within the freq. range, then we construct a one-to-one or
	// one-to-many mapping of control table slots to bins, arranged so that we
	// can control lower frequencies with greater resolution.
	//
	// Depending on the difference between the control table size and the number
	// of FFT bins, we create a mapping that begins linearly and grows
	// arithmetically after a certain point.  The purpose is to get higher
	// resolution in the lower frequencies of the range, where it matters most.
	// So near the low end of the range, there is a one-to-one mapping of control
	// array slots to FFT bins.  Near the high end of the range, one control
	// array slot affects many FFT bins.  We attempt to transition smoothly
	// between these two extremes.  As an example, the number of FFT bins
	// controlled by each slot of a control table array might look like this:
	//
	//            number of bins spanned by each cntl slot
	// (lowshelf) 1 1 1 1 1 1 1 1 1 1 1 2 3 4 5 6 7 8 9 10 14 (highshelf)
	//
	// The scheme isn't perfect -- e.g., the penultimate slot might control more
	// than you would expect -- but it does guarantee that each control table
	// slot affects at least one FFT bin.

	else {
		const int lowshelfbin = closest_bin(minfreq);
		const int highshelfbin = closest_bin(maxfreq);
		int cntltablen = control_table_size;
		const float endflatrange = minfreq + (cntltablen * _fund_anal_freq);
		bool linear_map = true;
		if (endflatrange > maxfreq - _fund_anal_freq) {
			const int ignorevals = int((endflatrange - maxfreq)
			                                                 / _fund_anal_freq);
			if (ignorevals > 0) {
				cntltablen = control_table_size - ignorevals;
				if (ignorevals != _prev_bg_ignorevals) {
					rtcmix_warn(instname(), "Control table of size %d too large for "
			                       "frequency range...ignoring last %d values",
			                       control_table_size, ignorevals);
					_prev_bg_ignorevals = ignorevals;
				}
			}
		}
		else
			linear_map = false;
#ifdef DEBUG_BINGROUPS
		printf("lowbin=%d, highbin=%d, endflatrange=%f, map=%s\n",
					lowshelfbin, highshelfbin, endflatrange,
					linear_map ? "linear" : "nonlinear");
#endif

		// assign low shelf group
		int bidx = 0;
		while (bidx <= lowshelfbin)
			bin_groups[bidx++] = 0;

		// assign interior groups
		if (linear_map) {
			int bingroup = 1;
			while (bidx < highshelfbin) {
				bin_groups[bidx++] = bingroup;
				if (bingroup < cntltablen - 2)
					bingroup++;
			}
		}
		else {	// higher array slots control more and more bins
			const int extrabins = (highshelfbin - lowshelfbin) - cntltablen;
			assert(extrabins > 0);
			const int nsums = int(sqrt(2 * extrabins) + 2);

			// Compute the sum of integers for n in [nsums-2, nsums);
			// formula is the closed-form equation: x = n * (n + 1) / 2.
			const int an = nsums - 2;
			const int bn = nsums - 1;
			const int a = (an * (an + 1)) / 2;
			const int b = (bn * (bn + 1)) / 2;
			int cntlspan = 0, binspan = 0;
			if (a > extrabins) {
				cntlspan = an;
				binspan = a;
			}
			else if (b > extrabins) {
				cntlspan = bn;
				binspan = b;
			}
			else
				assert(0);  // this should never happen, but if it does, we'll know

#ifdef DEBUG_BINGROUPS
			printf("nbins=%d, lowbin=%d, highbin=%d, tablen=%d, extrabins=%d, "
				"cntlspan=%d, binspan=%d\n\n",
				nbins, lowshelfbin, highshelfbin, cntltablen, extrabins,
				cntlspan, binspan);
#endif

			// first, linear mapping
			const int end = (cntltablen - cntlspan) - 1;
			int bingroup = 1;
			while (bingroup < end)
				bin_groups[bidx++] = bingroup++;

			// then map using sum-of-integers series
			int incr = 1;
			int count = 0;
			while (bidx < highshelfbin) {
				bin_groups[bidx++] = bingroup;
				if (++count == incr) {
					count = 0;
					if (bingroup < cntltablen - 2)
						bingroup++;
					incr++;
				}
			}
		}

		// assign high shelf group
		const int last = cntltablen - 1;
		while (bidx < nbins)
			bin_groups[bidx++] = last;
	}

	// print a user-readable listing of bin groups
	if (_print_stats) {
		printf("\n%s:  %s table bin groups\n", instname(), type);
		printf("------------------------------------------\n");
		int cntltabslot = 0;
		int startbin = 0;
		for (int i = 0; i < nbins; i++) {
			const int thisslot = bin_groups[i];
			if (thisslot > cntltabslot) {
				// print stats for previous control table slot <cntltabslot>
				const int startfreq = int(_fund_anal_freq * startbin + 0.5f);
				if (i - startbin > 1) {
					const int endfreq = int(_fund_anal_freq * (i - 1) + 0.5f);
					printf("  [%d]\t%d-%d Hz (%d bins)\n",
					          cntltabslot, startfreq, endfreq, i - startbin);
				}
				else
					printf("  [%d]\t%d Hz\n", cntltabslot, startfreq);
				cntltabslot = thisslot;
				startbin = i;
			}
		}
		// print stats for last control table slot
		const int startfreq = int(_fund_anal_freq * startbin + 0.5f);
		if (nbins - startbin > 1)
			printf("  [%d]\t%d-%d Hz (%d bins)\n\n",
						 cntltabslot, startfreq, int(_nyquist), nbins - startbin);
		else
			printf("  [%d]\t%d Hz\n\n", cntltabslot, startfreq);
		fflush(stdout);
	}

#ifdef DEBUG_BINGROUPS
	printf("bin_groups[] -----------------------------\n");
	for (int i = 0; i < nbins; i++)
		printf("[%d] %d (%f)\n", i, bin_groups[i], i * _fund_anal_freq);
#endif
#ifdef CHECK_BINGROUPS
	for (int i = 1; i < nbins; i++) {
		int diff = bin_groups[i] - bin_groups[i - 1];
		if (diff < 0 || diff > 1)
			printf("bin group (%p) index %d not 0 or 1 greater than prev entry\n",
			       bin_groups, i);
	}
#endif
}


// ----------------------------------------------------------------- doupdate --
// NB: Most pfields are updated in subclass, via subupdate().

void SPECTACLE2_BASE::doupdate()
{
	// We use the alternate update() method, because this pfield does not span
	// the entire note duration.  Note that if _iamp_pfield_index is 0, then the
	// subclass doesn't want an input amp envelope.

	if (_iamp_pfield_index && currentFrame() < _input_frames)
		_iamp = update(_iamp_pfield_index, _input_frames);
}


// ---------------------------------------------------------------------- run --
int SPECTACLE2_BASE::run()
{
	const int nframes = framesToRun();
	const int inchans = inputChannels();
	const int outchans = outputChannels();

	// If still taking input, store framesToRun() frames into <_inbuf>.
	const bool input_avail = (currentFrame() < _input_frames);
	const int insamps = nframes * inchans;
	if (input_avail)
		rtgetin(_inbuf, this, insamps);

	for (int i = 0; i < nframes; i++) {
		if (--_branch <= 0) {
			doupdate();
			subupdate();
			_branch = getSkip();
		}

		float insig;
		if (input_avail)
			insig = _inbuf[(i * inchans) + _inchan] * _iamp;
		else
			insig = 0.0f;
		_bucket->drop(insig);	// may process <_decimation> input frames

		float outsig = _outbuf[_out_read_index] * _wet;
		increment_out_read_index();

		float drysig = _dry_delay->next(insig);
		outsig += drysig * _dry;

		float out[outchans];
		out[0] = outsig * _oamp;
		if (outchans == 2) {
			out[1] = out[0] * (1.0f - _pan);
			out[0] *= _pan;
		}

		rtaddout(out);
		increment();
	}

	return nframes;
}


