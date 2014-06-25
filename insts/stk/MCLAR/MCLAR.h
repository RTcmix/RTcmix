class MCLAR : public Instrument {
	int nargs, branch;
	float amp, breathamp, pctleft;
	double *amptable;
	Ooscili *theEnv;
	Clarinet *theClar;
	double freq, noiseamp, stiff;
	void doupdate();

public:
	MCLAR();
	virtual ~MCLAR();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kFreq = 1 << 3,
	kNoise = 1 << 4,
	kStiff = 1 << 6,
	kPan = 1 << 7,
	kBreathPress = 1 << 8
};
