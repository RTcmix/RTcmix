class DUMP : public Instrument {
	int skip, branch;
	float amp;

	void doupdate();
public:
	DUMP();
	virtual ~DUMP();
	virtual int init(double p[], int n_args);
	virtual int configure();
	virtual int run();
};

