class AMINST : public Instrument {
	float amp,*amparr, amptabs[2];
	float *mamparr,mamptabs[2];
	float *cartable,sicar,phasecar;
	int lencar;
	float *modtable,simod,phasemod;
	int lenmod;
	float spread;
	int skip,inchan;

public:
	AMINST();
	int init(float*, int);
	int run();
	};
