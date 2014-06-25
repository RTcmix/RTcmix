class MBRASS : public Instrument {
	int nargs, branch;
	float amp, breathamp, pctleft;
	double *amptable;
	Ooscili *theEnv;
	Brass *theHorn;
	double freq, slength, lipfilt;
	void doupdate();

public:
	MBRASS();
	virtual ~MBRASS();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kFreq = 1 << 3,
	kSlide = 1 << 4,
	kLip = 1 << 5,
	kPan = 1 << 7,
	kBreathPress = 1 << 8
};
