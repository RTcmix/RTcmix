#include <Instrument.h>

class Ocomb;

class COMBIT : public Instrument {
	bool give_minfreq_warning;
	int insamps, branch, skip, inchan, delsamps;
	float amp, frequency, rvbtime, pctleft, *in, tabs[2];
	double *amptable;
	Ocomb *comb;

public:
	COMBIT();
	virtual ~COMBIT();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};
