#include <bus.h>        /* for MAXBUS */

class STEREO : public Instrument {
	float outspread[MAXBUS];
	float amp, aamp, *amptable, tabs[2], *in;
	int skip, branch;

public:
	STEREO();
	virtual ~STEREO();
	int init(double*, int);
	int run();
	};
