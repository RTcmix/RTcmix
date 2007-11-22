/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _ODCBLOCK_H_
#define _ODCBLOCK_H_ 1

// Simple DC-blocking filter, based on the method employed in STK.
// Default coefficient is .99, as it is in STK, but making this a little
// higher (say, .995, by calling setcoeff) will steepen the slope of the
// high-pass filter, reducing the impact on frequencies higher than DC.  -JGG

class Odcblock {

public:
	Odcblock();
	~Odcblock();

	inline float next(float input);
	float last() const { return _last; }
	void clear() { _hist = _last = 0.0; }
	void setcoeff(const float coeff) { _coeff = coeff; }

private:
	float _hist;
	float _last;
	float _coeff;
};

inline float Odcblock::next(float input)
{
	_last = input - _hist + (_coeff * _last);
	_hist = input;
	return _last;
}

#endif // _ODCBLOCK_H_
