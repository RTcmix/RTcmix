/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

class Ooscili
{
	double si, phase, lendivSR;
	double *array;
	float _sr;
	float dur;
	int length;

	void init(float);
public:
	Ooscili(float, float, int);
	Ooscili(float, float, double *);
	Ooscili(float, float, double *, int);
	float next();
	float next(int);
	inline void setfreq(float freq) { si = freq * lendivSR; }
	inline void setphase(double phs) { phase = phs; }
	inline int getlength() { return length; }
	inline float getdur() { return dur; }
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

class Ozdelay
{
public:
	Ozdelay(long maxLength);
	~Ozdelay();
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

class Ozcomb
{
public:
	Ozcomb(float SR, float loopTime, float maxLoopTime, float reverbTime);
	~Ozcomb();
	void clear();
	void setReverbTime(float reverbTime);
	float next(float input, float delaySamps);
private:
	Ozdelay *_delay;
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
