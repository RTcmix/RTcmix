class BUZZ : public Instrument {
	float myrsnetc[64][5],myamp[64];
	int mynresons;
	float oamp,*amparr,amptabs[2];
	float si,hn,phase,*sinetable;
	int skip;
	float spread;

public:
	BUZZ();
	int init(float*, short);
	int run();
	};
