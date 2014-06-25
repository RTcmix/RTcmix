#include "cfuncs.h"

class Oreson;

class PULSE : public Instrument {
	bool fastUpdate;
	int branch, nresons;
	float amp, ampmult, pan, si, phase, amptabs[2];
	double *amptable;
	Oreson *resons[MAXFILTER];
	float resonamp[MAXFILTER];

	void initamp(float dur, double p[], int ampindex, int ampgenslot);
	void doupdate();
	inline float mypulse(float amp);
public:
	PULSE();
	virtual ~PULSE();
	virtual int init(double *, int);
	virtual int run();
};


inline float PULSE::mypulse(float amp)
{
	phase += si;
	if (phase > 512.0f) {
		phase -= 512.0f;
		return amp;
	}
	return 0.0f;
}

