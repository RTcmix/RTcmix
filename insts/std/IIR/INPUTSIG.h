class INPUTSIG : public Instrument {
	float myrsnetc[64][5],myamp[64];
	int mynresons;
	float oamp, *amparr, amptabs[2], *in;
	int inchan;
	int skip;
	float spread;

public:
	INPUTSIG();
	virtual ~INPUTSIG();
	int init(double*, int);
	int run();
	};
