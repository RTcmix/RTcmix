// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Ougens.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
//#define NDEBUG
#include <assert.h>

const int kMaxFFTLen = 16384;

#ifndef powf
	#define powf(val, exp) pow((val), (exp))
#endif

#ifdef DEBUG
	#define DPRINT(msg) printf((msg))
	#define DPRINT1(msg, arg) printf((msg), (arg))
	#define DPRINT2(msg, arg1, arg2) printf((msg), (arg1), (arg2))
	#define DPRINT3(msg, arg1, arg2, arg3) printf((msg), (arg1), (arg2), (arg3))
#else
	#define DPRINT(msg)
	#define DPRINT1(msg, arg)
	#define DPRINT2(msg, arg1, arg2)
	#define DPRINT3(msg, arg1, arg2, arg3)
#endif

class SPECTACLE2_BASE : public Instrument {

public:
	SPECTACLE2_BASE();
	virtual ~SPECTACLE2_BASE();
	virtual int init(double p[], int n_args);
	virtual int configure();
	virtual int run();

protected:
	virtual int getargs(double p[], int n_args) = 0;
	virtual int subinit(double p[], int n_args) = 0;
	virtual int subconfigure() = 0;
	virtual void subupdate() = 0;
	virtual void modify_analysis(bool reading_input) = 0;
	virtual const char *instname() = 0;
	double *resample_functable(const double *table, const int oldsize,
	                                                const int newsize);
	inline int closest_bin(const float freq);
	void update_bin_groups(int bin_groups[], const float minfreq,
		const float maxfreq, const int control_table_size, const char *type);

	// helper functions
	float _ampdb(float db) { return powf(10.0f, db * 0.05f); }
	float _fclamp(float min, float val, float max) {
	                    return (val < min) ? min : ((val > max) ? max : val); }
	void _fswap(float *a, float *b) { float tmp = *a; *a = *b; *b = tmp; }
	int _imax(int x, int y) { return x > y ? x : y; }
	int _imin(int x, int y) { return x < y ? x : y; }
	int _odd(long x) { return (x & 0x00000001); }

	// accessors for subclasses
	void set_iamp_pfield_index(int index) { _iamp_pfield_index = index; }
	void set_window_pfield_index(int index) { _window_pfield_index = index; }
	void set_oamp(float oamp) { _oamp = oamp; }
	void set_ringdur(float ringdur) { _ringdur = ringdur; }
	void set_fftlen(int fftlen) { _fftlen = fftlen; }
	int get_fftlen() const { return _fftlen; }
	void set_window_len(int window_len) { _window_len = window_len; }
	int get_window_len() const { return _window_len; }
	void set_overlap(int overlap) { _overlap = overlap; }
	int get_overlap() const { return _overlap; }
	bool set_freqrange(float min, float max, const char *type = "delay");
	void set_wet(float wet) {
		if (wet != _wet) {
			_wet = _fclamp(0.0f, wet, 1.0f);
			_dry = 1.0f - wet;
		}
	}
	void set_inchan(int inchan) { _inchan = inchan; }
	void set_pan(float pan) { _pan = pan; }

	// For a subclass that wants to change the base class array for some reason.
	void set_bin_groups(int bin_groups[]) {
		delete [] _bin_groups;
		_bin_groups = bin_groups;	// assume new one is valid
	}
	void set_binmap_table(double *table) { _binmaptable = table; }

	int _input_frames, _nargs, _decimation, _overlap, _half_fftlen;
	float _inputdur, _fund_anal_freq, _nyquist;

	// <_anal_bins> stores magnitude and phase values for <half_fftlen> + 1
	// FFT bins.  (The first and last bins are for DC and Nyquist.)

	float *_anal_bins;

	// size of the "control tables" (e.g., EQ) used with _bin_groups[] below

	int _control_table_size;

	// A "bin group" is a range of FFT bins that are controlled by a single
	// element in one of the "control tables."  The <_bin_groups> array has
	// <half_fftlen> + 1 values, each of which is an index into these control
	// tables.  So <_bin_groups> tells us which control table value to use for
	// a given FFT bin.  <_bin_groups> is updated whenever minfreq or maxfreq
	// changes.  The base class _bin_groups array is used by a subclass in any
	// way it wants.  A subclass may have additional bin groups, but these are
	// handled entirely within the subclass.  The subclass can call
	// update_bin_groups to initialize these extra bin groups.

	int *_bin_groups;

private:
	int make_windows(const double *table, const int len);
	void prepare_input(const float buf[]);
	void cartesian_to_polar();
	void polar_to_cartesian();
	void prepare_output();
	static void process_wrapper(const float buf[], const int len, void *obj);
	void process(const float *buf, const int len);
	void doupdate();
	inline void increment_out_read_index();
	inline void increment_out_write_index();

	bool _print_stats;
	int _branch, _inchan, _fftlen, _window_len, _input_end_frame;
	int _prev_bg_ignorevals;
	int _out_read_index, _out_write_index, _outframes, _latency;
	int _iamp_pfield_index, _window_pfield_index, _window_len_minus_decimation;
	float _wet, _dry, _pan, _ringdur, _iamp, _oamp, _minfreq, _rawmaxfreq;
	float *_anal_window, *_synth_window, *_input, *_output, *_fft_buf;
	float *_inbuf, *_outbuf;
	double *_binmaptable;
	Offt *_fft;
	Obucket *_bucket;
	Odelay *_dry_delay;
};


// Return the index of the FFT bin whose center frequency is closest to <freq>.
inline int SPECTACLE2_BASE::closest_bin(const float freq)
{
	return int((freq / _fund_anal_freq) + 0.5f);
}


inline void SPECTACLE2_BASE::increment_out_read_index()
{
	if (++_out_read_index == _outframes)
		_out_read_index = 0;
//	assert(_out_read_index != _out_write_index);
}

inline void SPECTACLE2_BASE::increment_out_write_index()
{
	if (++_out_write_index == _outframes)
		_out_write_index = 0;
//	assert(_out_write_index != _out_read_index);
}


