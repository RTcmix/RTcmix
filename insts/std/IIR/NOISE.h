class NOISE : public Instrument {
	int skip, branch, mynresons;
	float myrsnetc[64][5], myamp[64];
	float oamp, spread, amptabs[2];
	double *amparr;

public:
	NOISE();
	virtual ~NOISE();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kPan = 1 << 3
};

