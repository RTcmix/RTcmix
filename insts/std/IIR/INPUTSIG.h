#include "cfuncs.h"

class Oreson;

class INPUTSIG : public Instrument {
	bool fastUpdate;
	int inchan, branch, nresons;
	float amp, ampmult, pan, *in, amptabs[2];
	double *amptable;
	Oreson *resons[MAXFILTER];
	float resonamp[MAXFILTER];

	void initamp(float dur, double p[], int ampindex, int ampgenslot);
	void doupdate();
public:
	INPUTSIG();
	virtual ~INPUTSIG();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 3,
	kPan = 1 << 5
};

