class MMODALBAR : public Instrument {
	float   amp, pctleft;
	double  amparray[2];
	Ooscili *theEnv;
	ModalBar *theBar;
	float excite[256];
	Orand *theRand;
	BiQuad *theFilt;

public:
	MMODALBAR();
	virtual ~MMODALBAR();
	virtual int init(double *, int);
	virtual int run();
};

