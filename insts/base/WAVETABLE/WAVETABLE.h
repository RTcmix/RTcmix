class WAVETABLE : public Instrument {
        float *wavetable,si,phase,amp;
        float *amptable,tabs[2];
        float spread;
        int len,alen;
	int skip;

public:
	WAVETABLE();
        int init(float p[], short n_args);
        int run();
        };
