#include <bus.h>        /* for MAXBUS */

class MIX : public Instrument {
	int outchan[MAXBUS];
	float amp,*in,*amptable,tabs[2];
	int skip;

public:
	MIX();
	virtual ~MIX();
	int init(float*, short);
	int run();
	};
