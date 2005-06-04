/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifdef FFTW
#include <fftw3.h>
#else
class FFTReal;
#endif

class Offt {
public:
	enum {
		kRealToComplex = 1,
		kComplexToReal = 2
	};

	Offt(int fftsize, unsigned int flags = kRealToComplex | kComplexToReal);
	~Offt();
	float *getbuf() const { return _buf; }
	void r2c();
	void c2r();

private:
	void printbuf(float *buf, char *msg);

	int _len;
	float *_buf;
#ifdef FFTW
	fftwf_complex *_cbuf;
	fftwf_plan _plan_r2c, _plan_c2r;
#else
	float *_tmp;
	FFTReal *_fftobj;
#endif
};

