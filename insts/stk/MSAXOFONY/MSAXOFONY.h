class MSAXOFONY : public Instrument {
	float   amp, pctleft;
	double  amparray[2];
	Ooscili *theEnv;
	Saxofony *theSax;

public:
	MSAXOFONY();
	virtual ~MSAXOFONY();
	virtual int init(double *, int);
	virtual int run();
};

