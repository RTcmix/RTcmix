#include <Ougens.h>

class PANECHO : public Instrument {
	bool warn_deltime;
	int inchan, insamps, skip, branch;
	double delsamps0, delsamps1;
	float amp, regen, prevdeltime0, prevdeltime1, *in, amptabs[2];
	double *amptable;
	Ozdelay *delay0, *delay1;

	double getdelsamps(float deltime);
public:
	PANECHO();
	virtual ~PANECHO();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 3,
	kDelTime0 = 1 << 4,
	kDelTime1 = 1 << 5,
	kDelRegen = 1 << 6
};

