class DELAY : public Instrument {
	float amp, *amptable, amptabs[2], *in;
	float *delarray;
	float wait,regen;
	float spread;
	int deltabs[2],inchan;
	int skip;
	int insamps;

public:
	DELAY();
	virtual ~DELAY();
	int init(double*, int);
	int run();
	};
