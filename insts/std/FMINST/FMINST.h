#include <Ougens.h>

class FMINST : public Instrument {
	int nargs, skip, branch;
	int lenind;
	float amp, carfreq, carfreqraw, modfreq, modfreqraw, peakdev, spread;
	double *indexenv, *ampenv;
	float indtabs[2], amptabs[2];
	Ooscili *carosc, *modosc;

	void doupdate();
public:
	FMINST();
	virtual ~FMINST();
	virtual int init(double *, int);
	virtual int run();
};
