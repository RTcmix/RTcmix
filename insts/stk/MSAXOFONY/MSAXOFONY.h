class MSAXOFONY : public Instrument {
	int nargs, branch;
	float amp, breathamp, pctleft;
	double *amptable;
	Ooscili *theEnv;
	Saxofony *theSax;
	double freq, noiseamp, stiff, aperture, blowpos;
	void doupdate();

public:
	MSAXOFONY();
	virtual ~MSAXOFONY();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kFreq = 1 << 3,
	kNoise = 1 << 4,
	kStiff = 1 << 6,
	kAperture = 1 << 7,
	kBlow = 1 << 8,
	kPan = 1 << 9,
	kBreathPress = 1 << 10
};
