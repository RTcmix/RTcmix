#include <Instrument.h>		  // the base class for this instrument

class MULTIFM : public Instrument {

public:
	MULTIFM();
	virtual ~MULTIFM();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
	virtual double fmodpos(double, int);

private:
	void doupdate();
	void initarrays(int);
	void createarrays(double *);

	int nargs, branch, numops, numouts, wavelen;
	double *phsinc, *outamps, *wavelentab, *phs, **allampsperop;
	int **allmodsperop, **allampsperopind, *nummodspercar, *outops, *opsnotout, *outampsind;
	double amp, pan;
	double *sinetable;
	Ooscili **oscil;
};

