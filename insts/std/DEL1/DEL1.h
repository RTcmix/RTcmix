class DEL1 : public Instrument {
	float amp, *amptable, amptabs[2], *in;
	float *delarray;
	float wait,delamp;
	int deltabs[2],inchan;
	int skip;
	int insamps;

public:
	DEL1();
	virtual ~DEL1();
	int init(float*, int);
	int run();
	};
