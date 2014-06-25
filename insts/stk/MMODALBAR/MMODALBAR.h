class MMODALBAR : public Instrument {
	int nargs, branch;
	float amp, pctleft;
	double *amptable;
	Ooscili *theEnv;
	ModalBar *theBar;
	float excite[256];
	Orand *theRand;
	BiQuad *theFilt;
	double freq, exciteamp;
	void doupdate();

public:
	MMODALBAR();
	virtual ~MMODALBAR();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kFreq = 1 << 3,
	kPan = 1 << 7,
	kAmpEnv = 1 << 8
};
