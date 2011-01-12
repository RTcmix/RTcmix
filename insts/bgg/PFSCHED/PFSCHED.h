#include <PField.h>

// each 'pfbus' is has the data in this structure.  It is declared
// here and referenced in src/control/pfbus/PFBusPField.cpp
struct pfbusdata {
	int drawflag;
	const PField *thepfield;
	double val;
	double percent;
	double theincr;
	int dqflag;
};

// this should be enough...
#define NPFBUSSES 1024
struct pfbusdata pfbusses[NPFBUSSES];
int pfbus_is_connected[NPFBUSSES];

class PFSCHED : public Instrument {
	int pfbus;
	int set_dq_flag;
	int firsttime;
	const PField *PFSCHEDpfield;

	void doupdate();

public:
	PFSCHED();
	virtual ~PFSCHED();
	virtual int init(double p[], int n_args);
	virtual int configure();
	virtual int run();
};
