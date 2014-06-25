#include <Ougens.h>

class DEL1 : public Instrument {
	bool warn_deltime;
	int inchan, insamps, skip, branch;
	float amp, delamp, *in, amptabs[2];
	double *amptable;
	double delsamps;
	Odelayi *delay;

public:
	DEL1();
	virtual ~DEL1();
	virtual int init(double*, int);
	virtual int configure();
	virtual int run();
};
