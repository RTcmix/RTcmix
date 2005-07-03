#include <Instrument.h>
#include <Ougens.h>

class MBANDEDWG : public Instrument {
	int nargs, branch, preset;
	float amp, velocity, pctleft;
	double *amptable, *veltable;
	double   velarray[2];
	Ooscili *theEnv;
	Ooscili *theVeloc;
	BandedWG *theBar;
	double freq, bowpress, modereson, integrate, strikepos, pluck, maxvelocity;
	void doupdate();

public:
	MBANDEDWG();
	virtual ~MBANDEDWG();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kFreq = 1 << 3,
	kBowPress = 1 << 8,
	kModeReson = 1 << 9,
	kIntegrate = 1 << 10,
	kPan = 1 << 11,
	kVel = 1 << 12
};
