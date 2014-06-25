class MBLOWHOLE : public Instrument {
	int nargs, branch;
	float amp, breathamp, pctleft;
	double *amptable;
	Ooscili *theEnv;
	BlowHole *theClar;
	double freq, noiseamp, stiff, tone, vent;
	void doupdate();

public:
	MBLOWHOLE();
	virtual ~MBLOWHOLE();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kFreq = 1 << 3,
	kNoise = 1 << 4,
	kStiff = 1 << 6,
	kTone = 1 << 7,
	kVent = 1 << 8,
	kPan = 1 << 9,
	kBreathPress = 1 << 10
};
