class WAVESHAPE : public Instrument {
	float *waveform, *ampenv, *xfer, *indenv;
	float amptabs[2],indtabs[2];
	float indbase,diff;
	int lenwave, lenxfer, lenind;
	float si,phs,amp;
	float a0,a1,b1,c,z1;
	float spread;

public:
	WAVESHAPE();
	int init(float*, short);
	int run();
	};
