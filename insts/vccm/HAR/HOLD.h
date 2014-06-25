class HOLD : public Instrument {
	float amp, tabs[2],*in;
	double *amptable, *fade_table;
	float aamp, f_tabs[2];
	float dur,spread;
	int branch;
	int skip, idx_samp, hold_samp, aud_idx, fade_samps, fade_samp;

public:
	HOLD();
	virtual ~HOLD();
	int init(double*, int);
	int configure();
	int run();
	};
