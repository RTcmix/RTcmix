class MCLAR : public Instrument {
	float   amp, pctleft;
	double  amparray[2];
	Ooscili *theEnv;
	Clarinet *theClar;

public:
	MCLAR();
	virtual ~MCLAR();
	virtual int init(double *, int);
	virtual int run();
};

