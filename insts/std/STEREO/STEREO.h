#include <bus.h>        /* for MAXBUS */

class STEREO : public Instrument {
	bool warnInvalid, fastUpdate;
	int branch, nargs, outslots;
	float outPan[MAXBUS];
	float amp, ampmult, *in, amptabs[2];
	double *amptable;

	void initamp(float dur, double p[], int ampindex, int ampgenslot);
	void updatePans(double p[]);
	void doupdate();
public:
	STEREO();
	virtual ~STEREO();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};
