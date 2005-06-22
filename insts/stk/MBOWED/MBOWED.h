class MBOWED : public Instrument {
	int nargs, branch;
	float amp, pctleft;
	double *amptable, *vibtable;
	Ooscili *theEnv;
	Ooscili *thePressure;
	Ooscili *thePosition;
	Ooscili *theVib;
	Bowed *theBow;
	int vibupdate;
	float freqbase, freqamp;
	float viblo, vibhi;
	Orand *theRand;
	void doupdate();
	double bowvel;

public:
	MBOWED();
	virtual ~MBOWED();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kFreq = 1 << 3,
	kVibDepth = 1 << 6,
	kPan = 1 << 7,
	kBowVel = 1 << 8,
	kBowPress = 1 << 9,
	kBowPos = 1 << 10
};
