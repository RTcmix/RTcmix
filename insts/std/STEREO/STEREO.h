class STEREO : public Instrument {
	float outspread[8];
	float amp,*amptable,tabs[2];
	int skip;

public:
	STEREO();
	int init(float*, short);
	int run();
	};
