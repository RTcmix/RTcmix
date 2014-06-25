class FIR : public Instrument {
	int ncoefs, branch, skip;
	float pastsamps[101];
	float coefs[101];
	float amp, *in;

public:
	FIR();
	virtual ~FIR();
	virtual int init(double *, int);
   virtual int configure();
	virtual int run();
};
