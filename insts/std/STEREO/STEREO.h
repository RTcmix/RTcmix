class STEREO : public Instrument {
	float outspread[8];
	float amp, *amptable, tabs[2], *in;
	int skip;

public:
	STEREO();
	virtual ~STEREO();
	int init(float*, short);
	int run();
	};
