class MBOWED : public Instrument {
	float   amp, pctleft;
	double   amparray[2];
	Ooscili *theEnv;
	Ooscili *thePressure;
	Ooscili *thePosition;
	Ooscili *theVib;
	Bowed *theBow;
	int vibupdate;
	float freqbase, freqamp;
	float viblo, vibhi;
	Orand *theRand;

public:
	MBOWED();
	virtual ~MBOWED();
	virtual int init(double *, int);
	virtual int run();
};

