#include <Instrument.h>

class HOLO : public Instrument {
	int ncoefs;
	int intap;
	float *pastsamps[2];
	float *pastsamps2[2];
	float amp, *in, *out;
	float xtalkAmp;
	int skip, count;

public:
	HOLO();
	virtual ~HOLO();
	int init(double*, int);
	int configure();
	int run();
};
