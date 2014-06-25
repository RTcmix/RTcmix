#include <Ougens.h>

class AM : public Instrument {
	int inchan, skip, branch;
	float *in, amp, modfreq, spread;
	float amptabs[2], freqtabs[2];
	double *wavetable, *amptable, *freqtable, *amtable;
	Ooscili *modosc;
	bool ownWavetable;

public:
	AM();
	virtual ~AM();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 3,
	kFreq = 1 << 4,
	kPan = 1 << 6
};

