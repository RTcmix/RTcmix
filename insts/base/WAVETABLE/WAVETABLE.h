#include <Ougens.h>

class WAVETABLE : public Instrument {
	int skip, branch;
	float amp, freqraw, spread;
	float *wavetable, *amptable, amptabs[2];
	Ooscili *osc;

	void doupdate(double p[]);
public:
	WAVETABLE();
	virtual ~WAVETABLE();
	virtual int init(double p[], int n_args);
	virtual int run();
};
