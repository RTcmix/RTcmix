class HOLD : public Instrument {
	float amp,*amptable,tabs[2],*in;
	float aamp, *fade_table, f_tabs[2];
	float dur,spread;
	int branch;
	int skip, idx_samp, hold_samp, aud_idx, fade_samps, fade_samp;

public:
	HOLD();
	virtual ~HOLD();
	int init(float*, int);
	int run();
	};
