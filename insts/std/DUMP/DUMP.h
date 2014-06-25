class DUMP : public Instrument {
	int nargs, skip, branch, tablelen;
	float amp;
	double *table;

	void doupdate();
public:
	DUMP();
	virtual ~DUMP();
	virtual int init(double p[], int n_args);
	virtual int configure();
	virtual int run();
};

