class MBRASS : public Instrument {
	float   amp, pctleft;
	double  amparray[2];
	Ooscili *theEnv;
	Brass *theHorn;

public:
	MBRASS();
	virtual ~MBRASS();
	virtual int init(double *, int);
	virtual int run();
};

