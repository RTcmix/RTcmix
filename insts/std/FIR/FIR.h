class FIR : public Instrument {
	int ncoefs;
	float pastsamps[101];
	float coefs[101];
	float amp, *in;

public:
	FIR();
	virtual ~FIR();
	int init(double*, int);
	int run();
	};
