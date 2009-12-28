#include <PField.h>

// each 'pfbus' is has the data in this structure.  It is declared
// here and referenced in src/control/pfbus/PFBusPfield.cpp
struct pfbusdata {
	int drawflag;
	const PField *thepfield;
	double val;
	double percent;
	double theincr;
};

// this should be enough...
#define NPFBUSSES 1024
struct pfbusdata pfbusses[NPFBUSSES];

class PFSCHED : public Instrument {
	int pfbus;
	int firsttime;

	void doupdate();

public:
	PFSCHED();
	virtual ~PFSCHED();
	virtual int init(double p[], int n_args);
	virtual int run();
};
