class FMINST : public Instrument {
	float sicar,simod,carphs,modphs;
	float *indexenv, *sine, *ampenv;
	int lensine, lenind;
	float indtabs[2], amptabs[2];
	float index, indbase, amp, diff;
	float spread;
	int skip;

public:
	FMINST();
	int init(float*, int);
	int run();
	};
