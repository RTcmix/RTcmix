/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <math.h>

class Ooscili
{
	double si, phase, lendivSR;
	double *array;
	float _sr;
	float dur;
	int length;

	void init(float);
public:
	Ooscili(float SR, float freq, int arr);
	Ooscili(float SR, float freq, double arr[]);
	Ooscili(float SR, float freq, double arr[], int len);
	float next();
	float next(int nsample);
	inline void setfreq(float freq) { si = freq * lendivSR; }
	inline void setphase(double phs) { phase = phs; }
	inline int getlength() { return length; }
	inline float getdur() { return dur; }
};


class Oonepole
{
public:
	Oonepole(float SR);
	Oonepole(float SR, float freq);
	void setfreq(float freq);
	inline void clear() { _hist = 0.0; }

	inline void setpole(float coeff)
	{
		_b = coeff;
		_a = (_b > 0.0) ? 1.0 - _b : 1.0 + _b;
	}

	inline float next(float input)
	{
		_hist = (_a * input) + (_b * _hist);
		return _hist;
	}

private:
	float _sr;
	float _hist;
	float _a;
	float _b;
};


typedef enum {
   OeqLowPass = 0,
   OeqHighPass,
   OeqBandPassCSG,    // CSG: constant skirt gain; peak gain = Q
   OeqBandPassCPG,    // CPG: constant 0 dB peak gain
   OeqBandPass = OeqBandPassCPG,
   OeqNotch,
   OeqAllPass,
   OeqPeaking,
   OeqLowShelf,
   OeqHighShelf,
   OeqInvalid
} OeqType;

class Oequalizer
{
public:
	Oequalizer(float SR, OeqType type);
   void settype(OeqType type) { _type = type; }
	void setparams(float freq, float Q, float gain = 0.0);
	inline void clear() { _x1 = _x2 = _y1 = _y2 = 0.0; }

	inline float next(float input)
	{
		double y0 = (_c0 * input) + (_c1 * _x1) + (_c2 * _x2)
										  - (_c3 * _y1) - (_c4 * _y2);
		_x2 = _x1;
		_x1 = input;
		_y2 = _y1;
		_y1 = y0;

		return y0;
	}

private:
	double _sr;
	OeqType _type;
	double _c0, _c1, _c2, _c3, _c4;
	double _x1, _x2, _y1, _y2;
};


class Orand
{
	long rand_x;

public:
	Orand();
	Orand(int);
	void seed(int);
	void timeseed();
	float random();
	float rand();
	float range(float, float);
};


class Odelayi
{
public:
	Odelayi(long maxLength);
	~Odelayi();
	void clear();
	void putsamp(float samp);
	float getsamp(double lagsamps);
	void setdelay(double lagsamps);
	float next(float input);
	float last() { return _lastout; }
private:
	float *_dline;
	long _maxlen;
	long _inpoint;
	long _outpoint;
	double _frac;
	float _lastout;
};


class Ocomb
{
public:
	Ocomb(float SR, float loopTime, float reverbTime);
	Ocomb(float SR, float loopTime, float maxLoopTime, float reverbTime);
	~Ocomb();
	void clear();
	void setReverbTime(float reverbTime);
	float next(float input);
	float next(float input, int delaySamps);
private:
	void init(float loopTime, float maxLoopTime, float reverbTime);
	float _sr;
	float *_dline;
	int _len;
	int _delsamps;
	float _gain;
	int _pointer;
};


class Ocombi
{
public:
	Ocombi(float SR, float loopTime, float maxLoopTime, float reverbTime);
	~Ocombi();
	void clear();
	void setReverbTime(float reverbTime);
	float next(float input, float delaySamps);
private:
	Odelayi *_delay;
	float _sr;
	float _gain;
	float _lastout;
	float _delsamps;
};


class Instrument;

class Ortgetin
{
	Instrument *theInst;
	int chns, rsamps, chptr;
	float *in;

public:
	Ortgetin(Instrument*);
	int next(float*);
};
