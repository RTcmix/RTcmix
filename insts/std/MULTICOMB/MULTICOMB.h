#include <Ougens.h>

#define NCOMBS 4

class MULTICOMB : public Instrument {
	int inchan, insamps, branch, delsamps[NCOMBS];
	float amp, rvbtime, *in, spread[NCOMBS], amptabs[2];
	double *amptable;
	Ocomb *comb[NCOMBS];

public:
	MULTICOMB();
	virtual ~MULTICOMB();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 3,
	kRvbTime = 1 << 6
};

