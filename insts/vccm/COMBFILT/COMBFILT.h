#include <Instrument.h>

typedef enum {
  FIR=0,
  IIR
} f_type;

class COMBFILT : public Instrument {
	int insamps;
	float amp, tabs[2], *in;
	double *amptable;
	int skip,inchan;
	float spread;
	float a,b, combfreq,wetdry;
	float *x, *y;
	int type;
	int delay, maxdelay, runsamp, delaysamp;
	
	
public:
	COMBFILT();
	virtual ~COMBFILT();
	int init(double*, int);
	int configure();
	int run();
};
