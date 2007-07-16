class VWAVE : public Instrument {
	float amp, spread;
	Ooscili **theOscils;
	double *divpoints, vecdex;
	int ndivs;
	int branch;

	void doupdate();

public:
	VWAVE();
	virtual ~VWAVE();
	virtual int init(double p[], int n_args);
	virtual int run();
};
