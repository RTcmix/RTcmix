#include "cfuncs.h"

class Oreson;

class IINOISE : public Instrument {
	bool fastUpdate;
	int branch, nresons;
	float amp, ampmult, pan, amptabs[2];
	double *amptable;
	Oreson *resons[MAXFILTER];
	float resonamp[MAXFILTER];

	void initamp(float dur, double p[], int ampindex, int ampgenslot);
	void doupdate();
public:
	IINOISE();
	virtual ~IINOISE();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kPan = 1 << 3
};

