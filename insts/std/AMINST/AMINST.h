#include <Ougens.h>

class AMINST : public Instrument {
	int skip, branch, inchan;
	float amp, spread, modamp, amptabs[2], modamptabs[2];
	double *amparr, *modamparr, *cartable, *modtable;
	Ooscili *carosc, *modosc;
	bool ownModtable, ownCartable;
	bool p6present; // this is to accomodate default, and ordering of pfields

	void doupdate();
public:
	AMINST();
	virtual ~AMINST();
	virtual int init(double *, int);
	virtual int run();
};

