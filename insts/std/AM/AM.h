class AM : public Instrument {
	float amp, npoints, *in;
	float *amptable, amptabs[2];
	float *freqtable, freqtabs[2];
	float *amtable,si,phase;
	int lenam;
	float spread;
	int skip,inchan;

public:
	AM();
	virtual ~AM();
	int init(double*, int);
	int run();
	};
