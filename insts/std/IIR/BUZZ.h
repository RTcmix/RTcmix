class BUZZ : public Instrument {
	bool our_sine_table;
	int skip, branch, mynresons, lensine;
	float myrsnetc[64][5], myamp[64];
	float oamp, spread, prevpitch, si, hn, phase;
	float amptabs[2];
	double *amparr, *sinetable;

	void setpitch(float);
public:
	BUZZ();
	virtual ~BUZZ();
	virtual int init(double *, int);
	virtual int run();
};
