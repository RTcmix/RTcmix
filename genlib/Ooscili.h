/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _OOSCILI_H_
#define _OOSCILI_H_ 1

#include <stdint.h>

// We use a 16.16 fixed-point integer for phase calculations.  65536 (2^16)
// is the amount to shift each half of the integer (whole and decimal).
// NOTE: we could double the size of this on 64-bit platforms, but be
// careful of overflow when multiplying by <frac> in next().   If changing,
// note macros at top of .cpp file.

typedef int32_t fixed_t;	// 16.16 fixed point

inline fixed_t fp(double val) { return (fixed_t) (val * 65536); }

class Ooscili
{
	double lendivSR;
	double tabscale;
	fixed_t si, phase;
	double *array;
	float _sr;
	int length;

	void init(float);
public:
	Ooscili(float SR, float freq, int arr);
	Ooscili(float SR, float freq, double arr[], int len);
	float next();
	float next(int nsample);
	inline void setfreq(float freq) { si = fp(freq * lendivSR); }
	inline void setphase(double phs) { phase = fp(phs); }
	inline double getphase() const { return double(phase) / 65536; }
	inline int getlength() const { return length; }
//	inline float getdur() const { return 1.0 / freq; }
};

#endif // _OOSCILI_H_
