#include <bus.h>        /* for MAXBUS */

class STEREO : public Instrument {
	bool warn_invalid;
	int skip, branch, nargs, outslots;
	float outspread[MAXBUS];
	float amp, *in;
	float *amptable, tabs[2];

	void updatePans(double p[]);
public:
	STEREO();
	virtual ~STEREO();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};
