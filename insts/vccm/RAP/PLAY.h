class PLAY : public Instrument {
	float amp,*amptable,tabs[2],*in;
	float dur;
	int skip, idx_samp, aud_idx;

public:
	PLAY();
	virtual ~PLAY();
	int init(float*, short);
	int run();
	};
