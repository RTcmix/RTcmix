#include <Instrument.h>

class MIX : public Instrument {
	int branch, outchan[MAXBUS];
	bool fastUpdate;
	float amp, ampmult, *in, amptabs[2];
	double *amptable;

	void initamp(float dur, double p[], int ampindex, int ampgenslot);
public:
	MIX();
	virtual ~MIX();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};

