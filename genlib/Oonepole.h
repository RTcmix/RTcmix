/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _OONEPOLE_H_
#define _OONEPOLE_H_ 1

// Adapted from Dodge and Jerse, and STK OnePole, by JGG

class Oonepole
{
public:
	Oonepole(float SR);
	Oonepole(float SR, float freq);

	// Set initial value of history sample (0 by default).

	inline void sethist(float hist) { _hist = hist; }

	inline void clear() { _hist = 0.0; }

	// Positive freq gives lowpass; negative freq gives highpass.

	void setfreq(float freq);

	// Alternative to setfreq, designed for control rate signals.
	// Lag should be in range [0, 1].

	void setlag(float lag);

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


// OonepoleTrack

class OonepoleTrack : public Oonepole
{
public:
	OonepoleTrack(float SR);
	void setfreq(float freq);
	void setlag(float lag);
private:
	float _freq;
	float _lag;
};

#endif // _OONEPOLE_H_
