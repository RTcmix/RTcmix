class AMINST : public Instrument {
	float amp, amptabs[2], mamptabs[2];
	double *amparr, *mamparr, *cartable, *modtable;
	float sicar,phasecar,simod,phasemod;
	int lencar;
	int lenmod;
	float spread;
	int skip,inchan;

public:
	AMINST();
	int init(double*, int);
	int run();
	};
