class FADE_HOLD : public Instrument {
	float dur;

public:
	FADE_HOLD();
	virtual ~FADE_HOLD();
	int init(float*, int);
	int run();
	};
