class INPUTSIG : public Instrument {
	float myrsnetc[64][5],myamp[64];
	int mynresons;
	float oamp,*amparr,amptabs[2];
	int inchan;
	int skip;
	float spread;

public:
	INPUTSIG();
	int init(float*, short);
	int run();
	};
