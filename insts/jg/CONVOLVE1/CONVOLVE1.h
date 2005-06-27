#include <Instrument.h>

class Obucket;
class Offt;
class Ooscili;

class CONVOLVE1 : public Instrument {
public:
	CONVOLVE1();
	virtual ~CONVOLVE1();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	int prepareImpulse();
	void convolve();
	static void processWrapper(const float buf[], const int len, void *obj);
	void process(const float *buf, const int len);
	void doupdate();
	inline void incrementOutReadIndex();
	inline void incrementOutWriteIndex();

	int _branch, _inchan, _imptablen, _impframes, _inframes, _outframes;
	int _impStartIndex, _halfFFTlen, _fftlen, _outReadIndex, _outWriteIndex;
	float _impgain, _amp, _wetpct, _pan;
	float *_inbuf, *_fftbuf, *_imp, *_ovadd, *_dry, *_wet;
	double *_imptab;
	Obucket *_bucket;
	Offt *_fft;
	Ooscili *_winosc;
};


#define NDEBUG
#include <assert.h>

inline void CONVOLVE1::incrementOutReadIndex()
{
	if (++_outReadIndex == _outframes)
		_outReadIndex = 0;
	assert(_outReadIndex != _outWriteIndex);
}

inline void CONVOLVE1::incrementOutWriteIndex()
{
	if (++_outWriteIndex == _outframes)
		_outWriteIndex = 0;
	assert(_outWriteIndex != _outReadIndex);
}

