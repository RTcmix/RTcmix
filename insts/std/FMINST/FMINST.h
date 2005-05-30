#include <Ougens.h>

class FMINST : public Instrument {
	int nargs, branch, lenind;
	bool fastUpdate;
	float amp, ampmult, carfreq, carfreqraw, modfreq, modfreqraw, peakdev, pan;
	float minindex, indexdiff;
	double *indexenv, *amptable;
	float indtabs[2], amptabs[2];
	Ooscili *carosc, *modosc;

	void initamp(float dur, double p[], int ampindex, int ampgenslot);
	void doupdate();
public:
	FMINST();
	virtual ~FMINST();
	virtual int init(double *, int);
	virtual int run();
};
