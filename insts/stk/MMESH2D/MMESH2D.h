class MMESH2D : public Instrument {
	float   amp, pctleft;
	double  amparray[2];
	Ooscili *theEnv;
	Mesh2D *theMesh;

public:
	MMESH2D();
	virtual ~MMESH2D();
	virtual int init(double *, int);
	virtual int run();
};

