#include <Ougens.h>

class WAVETABLE : public Instrument {
	int branch;
	bool fastUpdate, ownWavetable;
	float amp, ampmult, freqraw, spread, amptabs[2];
	double *wavetable, *amptable;
	Ooscili *osc;

	void initamp(float dur, double p[], int ampindex, int ampgenslot);
	void doupdate();
public:
	WAVETABLE();
	virtual ~WAVETABLE();
	virtual int init(double p[], int n_args);
	virtual int run();
};
