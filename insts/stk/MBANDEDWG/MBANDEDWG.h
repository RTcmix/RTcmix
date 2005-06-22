#include <Ougens.h>

class MBANDEDWG : public Instrument {
	int nargs, branch;
	float amp, velocity, pctleft;
	double *amptable, *veltable;
	double   velarray[2];
	Ooscili *theEnv;
	Ooscili *theVeloc;
	BandedWG *theBar;
	double bowpress, modereson, integrate;
	void doupdate();

public:
	MBANDEDWG();
	virtual ~MBANDEDWG();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kBowPress = 1 << 8,
	kModeReson = 1 << 9,
	kIntegrate = 1 << 10,
	kPan = 1 << 11,
	kVel = 1 << 12
};
