#include <Ougens.h>

class DELAY : public Instrument {
	bool warn_deltime;
	double delsamps;
	float amp, *in, pctleft, regen, amptabs[2];
	double *amptable;
	int inchan, insamps, skip, branch;
	Odelayi *delay;

public:
	DELAY();
	virtual ~DELAY();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 3,
	kDelTime = 1 << 4,
	kDelRegen = 1 << 5,
	kPan = 1 << 8
};

