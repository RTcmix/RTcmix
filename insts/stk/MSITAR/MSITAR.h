class MSITAR : public Instrument {
	float   amp, pctleft;
	double  amparray[2];
	Ooscili *theEnv;
	Sitar *theSitar;

public:
	MSITAR();
	virtual ~MSITAR();
	virtual int init(double *, int);
	virtual int run();
};

