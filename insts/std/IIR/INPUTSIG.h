class INPUTSIG : public Instrument {
	int inchan, skip, branch, mynresons;
	float myrsnetc[64][5], myamp[64];
	float oamp, spread, *in;
	float *amparr, amptabs[2];

public:
	INPUTSIG();
	virtual ~INPUTSIG();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 3,
	kPan = 1 << 5
};

