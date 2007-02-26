// RTcmix - Copyright (C) 2005  The RTcmix Development Team
// See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
// the license to this software and for a DISCLAIMER OF ALL WARRANTIES.

// Translation of cmix reson.  The only thing different is the scaling factor,
// which now accords with earlier cmix man pages and with csound.  -JGG

class Oreson {
public:
	typedef enum {
		kNoScale = 0,	// no scaling of signal
		kPeakResponse,	// peak response factor of 1; use for harmonic signals
		kRMSResponse	// RMS response factor of 1; use for white noise signals
	} Scale;

	Oreson(float srate, float centerFreq, float bandwidth,
	                                            Scale scale = kPeakResponse);
	void setparams(float cf, float bw);
	inline void clear();
	inline float next(float sig);
	float last() { return _last; }

private:
	float _srate;
	Scale _scale;
	float _a0;
	float _a1;
	float _a2;
	float _h0;
	float _h1;
	float _last;
};

inline void Oreson::clear()
{
	_h0 = _h1 = 0.0f;
}

inline float Oreson::next(float sig)
{
	_last = (_a0 * sig) + (_a1 * _h0) - (_a2 * _h1);
	_h1 = _h0;
	_h0 = _last;
	return _last;
}

