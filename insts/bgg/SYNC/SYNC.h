class SYNC : public Instrument {
	float amp, spread;
	Ooscili *theOscil;
	double reset_samps, sample_count;
	int branch;

	void doupdate();

public:
	SYNC();
	virtual ~SYNC();
	virtual int init(double p[], int n_args);
	virtual int run();
};
