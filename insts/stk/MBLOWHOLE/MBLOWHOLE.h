class MBLOWHOLE : public Instrument {
	float   amp, pctleft;
	double  amparray[2];
	Ooscili *theEnv;
	BlowHole *theClar;

public:
	MBLOWHOLE();
	virtual ~MBLOWHOLE();
	virtual int init(double *, int);
	virtual int run();
};

