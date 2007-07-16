class HALFWAVE : public Instrument {
	float amp, spread;
	Ooscili *theOscils[2];
	double divpoint, endpoint, sample_count;
	int wavelens[2];
	int oscnum;
	int branch;

	void doupdate();

public:
	HALFWAVE();
	virtual ~HALFWAVE();
	virtual int init(double p[], int n_args);
	virtual int run();
};
