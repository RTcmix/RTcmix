class WAVETABLE : public Instrument {
   float *wavetable,si,phase,amp,aamp;
   float *amptable,tabs[2];
   float spread;
   int len,alen;
	int skip, branch;

public:
	WAVETABLE();
        int init(double p[], int n_args);
        int run();
        };
