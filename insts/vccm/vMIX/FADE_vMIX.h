class FADE_vMIX : public Instrument {
	float dur;

public:
	FADE_vMIX();
	virtual ~FADE_vMIX();
	int init(float*, int);
	int run();
	};
