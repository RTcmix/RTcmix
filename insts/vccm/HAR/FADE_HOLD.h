class FADE_HOLD : public Instrument {
	float dur;

public:
	FADE_HOLD();
	virtual ~FADE_HOLD();
	int init(double*, int);
	int run();
	};
