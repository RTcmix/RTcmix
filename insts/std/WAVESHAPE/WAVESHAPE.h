class WAVESHAPE : public Instrument {
	double *waveform, *ampenv, *xfer, *indenv;
	float amptabs[2],indtabs[2];
	float indbase,diff;
	int lenwave, lenxfer, lenind;
	float si,phs,amp;
	float a0,a1,b1,c,z1;
	float spread;
	int skip;

public:
	WAVESHAPE();
	int init(double*, int);
	int run();
	};
