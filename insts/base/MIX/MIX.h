#include <Instrument.h>

class MIX : public Instrument {
	int outchan[MAXBUS];
	float amp,aamp,*in,*amptable,tabs[2];
	int skip, branch;

public:
	MIX();
	virtual ~MIX();
	int init(double*, int);
	int configure();
	int run();
	};
