/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _ODCBLOCK_H_
#define _ODCBLOCK_H_ 1

// Simple DC-blocking filter, based on the method employed in STK.  -JGG

class Odcblock {

public:
	Odcblock();
	~Odcblock();

	inline float next(float input);
	float last() const { return _last; }
	void clear() { _hist = _last = 0.0; }

private:
	float _hist;
	float _last;
};

inline float Odcblock::next(float input)
{
	_last = input - _hist + (0.99f * _last);
	_hist = input;
	return _last;
}

#endif // _ODCBLOCK_H_
