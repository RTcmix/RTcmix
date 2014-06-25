#include "cfuncs.h"

class Oreson;

class BUZZ : public Instrument {
	bool our_sine_table, fastUpdate;
	int branch, nresons, lensine;
	float amp, ampmult, pan, prevpitch, si, hn, phase;
	float amptabs[2];
	double *amptable, *sinetable;
	Oreson *resons[MAXFILTER];
	float resonamp[MAXFILTER];

	void initamp(float dur, double p[], int ampindex, int ampgenslot);
	void doupdate();
	void setpitch(float);
public:
	BUZZ();
	virtual ~BUZZ();
	virtual int init(double *, int);
	virtual int run();
};
