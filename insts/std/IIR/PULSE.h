class PULSE : public Instrument {
	float myrsnetc[64][5],myamp[64];
	int mynresons;
	float oamp,*amparr,amptabs[2];
	float si,phase;
	int skip;
	float spread;

public:
	PULSE();
	virtual ~PULSE();
	int init(float*, int);
	int run();
	};
