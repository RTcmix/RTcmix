#include <Instrument.h>

class MIX : public Instrument {
	int outchan[MAXBUS];
	float amp, *in;
	float *amptable, tabs[2];
	int skip, branch;

public:
	MIX();
	virtual ~MIX();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};

