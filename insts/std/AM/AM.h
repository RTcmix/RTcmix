class AM : public Instrument {
	float amp, npoints, *in;
	float amptabs[2], freqtabs[2];
	double *amptable, *freqtable, *amtable;
	float si,phase;
	int lenam;
	float spread;
	int skip,inchan;

public:
	AM();
	virtual ~AM();
	int init(double*, int);
	int run();
	};
