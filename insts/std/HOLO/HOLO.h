#include <Instrument.h>

class HOLO : public Instrument {
	int ncoefs;
	int intap;
	float *pastsamps[2];
	float *pastsamps2[2];
	float amp, *in, *out;
	float xtalkAmp;

public:
	HOLO();
	virtual ~HOLO();
	int init(float*, int);
	int run();
};
