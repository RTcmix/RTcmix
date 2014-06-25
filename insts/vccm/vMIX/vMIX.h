#include <Instrument.h>
#include <bus.h>        /* for MAXBUS */

class vMIX : public Instrument {
	int outchan[MAXBUS];
	float amp,*in, tabs[2];
	double *amptable, *fade_table;
	float aamp, f_tabs[2];
	int skip, fade_samps, fade_samp, branch;
	int fade_started;

public:
	vMIX();
	virtual ~vMIX();
	int init(double*, int);
	int configure();
	int run();
	};
