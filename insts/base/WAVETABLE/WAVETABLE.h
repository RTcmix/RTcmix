class WAVETABLE : public Instrument {
   float *wavetable,si,phase,amp;
   float *amptable,tabs[2];
   float spread;
   int len,alen;
	int skip, branch;

public:
	WAVETABLE();
        int init(float p[], int n_args);
        int run();
        };
