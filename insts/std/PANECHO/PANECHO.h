class PANECHO : public Instrument {
	float amp, *amptable, amptabs[2], *in;
	float *delarray1,*delarray2;
	float wait1,wait2,regen;
	int deltabs1[2],deltabs2[2],inchan;
	int skip;
	int insamps;

public:
	PANECHO();
	virtual ~PANECHO();
	int init(float*, int);
	int run();
	};
