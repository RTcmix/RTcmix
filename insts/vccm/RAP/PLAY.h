class PLAY : public Instrument {
	float amp, tabs[2],*in;
	double *amptable;
	float dur;
	int skip, idx_samp, aud_idx;

public:
	PLAY();
	virtual ~PLAY();
	int init(double*, int);
	int configure();
	int run();
	};
