#include <bus.h>        /* for MAXBUS */

class vMIX : public Instrument {
	int outchan[MAXBUS];
	float amp,*in,*amptable,tabs[2];
	float aamp, *fade_table, f_tabs[2];
	int skip, fade_samps, fade_samp, branch;
	int fade_started;

public:
	vMIX();
	virtual ~vMIX();
	int init(float*, int);
	int run();
	};
