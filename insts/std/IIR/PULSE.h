class PULSE : public Instrument {
	int skip, branch, mynresons;
	float myrsnetc[64][5], myamp[64];
	float oamp, spread, si, phase;
	float *amparr, amptabs[2];

public:
	PULSE();
	virtual ~PULSE();
	virtual int init(double *, int);
	virtual int run();
};

