class MSITAR : public Instrument {
	int nargs, branch;
	float amp, pctleft;
	double *amptable;
	Ooscili *theEnv;
	Sitar *theSitar;
	double stramp, freq;

public:
	MSITAR();
	virtual ~MSITAR();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kFreq = 1 << 3,
	kPan = 1 << 5,
	kStramp = 1 << 6
};
