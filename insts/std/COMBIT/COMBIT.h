#include <Ougens.h>

class COMBIT : public Instrument {
	bool give_minfreq_warning;
	int insamps, branch, skip, inchan, delsamps;
	float amp, frequency, rvbtime, pctleft, *in;
	float *amptable, tabs[2];
	Ocomb *comb;

public:
	COMBIT();
	virtual ~COMBIT();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};
