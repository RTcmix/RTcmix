/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _OOSCILI_H_
#define _OOSCILI_H_ 1

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

#endif // _OOSCILI_H_
