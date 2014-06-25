#include <Instrument.h>

class Odelay;

class DELAY : public Instrument {
	double delsamps;
	float amp, *in, pctleft, regen, amptabs[2];
	double *amptable;
	int inchan, insamps, branch;
	Odelay *delay;
	// update flags (shift amount is pfield number)
	enum {
		kDelTime = 1 << 4,
		kDelRegen = 1 << 5,
		kPan = 1 << 8
	};
public:
	DELAY();
	virtual ~DELAY();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};

