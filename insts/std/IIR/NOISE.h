class NOISE : public Instrument {
	float myrsnetc[64][5],myamp[64];
	int mynresons;
	float oamp,*amparr,amptabs[2];
	int skip;
	float spread;

public:
	NOISE();
	virtual ~NOISE();
	int init(double*, int);
	int run();
	};
