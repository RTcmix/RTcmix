#include <Ougens.h>

class FMINST : public Instrument {
	int nargs, skip, branch;
	int lenind;
	float amp, carfreq, carfreqraw, modfreq, modfreqraw, peakdev, spread;
	float *indexenv, *wavetable, *ampenv;
	float indtabs[2], amptabs[2];
	Ooscili *carosc, *modosc;

	void doupdate(double p[]);
public:
	FMINST();
	virtual ~FMINST();
	virtual int init(double *, int);
	virtual int run();
};
