class PFSCHED : public Instrument {
	int pfbus;

	int makedyntable();
	void doupdate();

public:
	PFSCHED();
	virtual ~PFSCHED();
	virtual int init(double p[], int n_args);
	virtual int configure();
	virtual int run();
};
