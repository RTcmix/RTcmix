class AM : public Instrument {
	float amp, *amptable, amptabs[2], *in;
	float *amtable,si,phase;
	int lenam;
	float spread;
	int skip,inchan;

public:
	AM();
	virtual ~AM();
	int init(float*, short);
	int run();
	};
