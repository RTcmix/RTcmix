#include <Ougens.h>

class WAVESHAPE : public Instrument {
	bool doampnorm;
	int nargs, skip, branch;
	int lenwave, lenxfer, lenind;
	float amptabs[2], indtabs[2];
	float amp, rawfreq, spread, index;
	float a0, a1, b1, z1;
	double *waveform, *ampenv, *xferfunc, *indenv;
	Ooscili *osc;

	void setDCBlocker(float freq, bool init);
	void doupdate();
public:
	WAVESHAPE();
	virtual ~WAVESHAPE();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield index)
enum {
	kFreq = 1 << 2,
	kMinIndex = 1 << 3,
	kMaxIndex = 1 << 4,
	kAmp = 1 << 5,
	kPan = 1 << 6,
	kIndex = 1 << 9
};

