// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include <Instrument.h>
#include <fparser27/fparser.hh>

class Ooscil;

class WAVY : public Instrument {
	typedef float (*CombineFunction)(float, float);
	static inline float a(float, float);
	static inline float b(float, float);
	static inline float add(float, float);
	static inline float subtract(float, float);
	static inline float multiply(float, float);
	void setCombineFunc(CombineFunction func) { _combiner = func; }

	inline float eval(float, float);

	int _nargs, _branch;
	float _amp, _freqA, _freqB, _pan;
	double _freqAraw, _freqBraw, _phaseOffset;
	Ooscil *_oscilA, *_oscilB;
	CombineFunction _combiner;
	FunctionParser *_fp;

	int usage() const;
	int setExpression();
	void doupdate();

public:
	WAVY();
	virtual ~WAVY();
	virtual int init(double p[], int n_args);
	virtual int run();
};

inline float WAVY::a(float a, float) { return a; }		// ignore <b>
inline float WAVY::b(float, float b) { return b; }		// ignore <a>
inline float WAVY::add(float a, float b) { return a + b; }
inline float WAVY::subtract(float a, float b) { return a - b; }
inline float WAVY::multiply(float a, float b) { return a * b; }

#include <math.h>		// for isnan()

inline float WAVY::eval(float a, float b)
{
	double vars[] = { a, b };
	float val = _fp->Eval(vars);
	if (isnan(val) || _fp->EvalError())
		val = 0.0f;
	return val;
}

