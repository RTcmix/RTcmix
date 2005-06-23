class MSHAKERS : public Instrument {
	int nargs, branch;
	float amp, aamp, pctleft;
	Shakers *theShake;
	double energy, decay, nobjects, resfreq;
	void doupdate();

public:
	MSHAKERS();
	virtual ~MSHAKERS();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kEnergy = 1 << 3,
	kDecay = 1 << 4,
	kNobjs = 1 << 5,
	kRfreq = 1 << 6,
	kPan = 1 << 8
};
