class MSHAKERS : public Instrument {
	float   amp, pctleft;
	Shakers *theShake;

public:
	MSHAKERS();
	virtual ~MSHAKERS();
	virtual int init(double *, int);
	virtual int run();
};

