class COMBIT : public Instrument {
	int insamps;
	float *combarr;
	float amp, *amptable, tabs[2];
	int skip,inchan;
	float spread;

public:
	COMBIT();
	virtual ~COMBIT();
	int init(float*, short);
	int run();
};
