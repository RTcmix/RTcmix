#include <Ougens.h>

class WAVETABLE : public Instrument {
	int skip, branch;
	float amp, ampinc, freqraw, spread, amptabs[2];
	double *wavetable, *amptable;
	Ooscili *osc;

	void doupdate();
public:
	WAVETABLE();
	virtual ~WAVETABLE();
	virtual int init(double p[], int n_args);
	virtual int run();
};
