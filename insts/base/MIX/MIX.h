class MIX : public Instrument {
	int outchan[8];
	float amp,*amptable,tabs[2];
	int skip;

public:
	MIX();
	int init(float*, short);
	int run();
	};
