#include <bus.h>        /* for MAXBUS */

class MIXN : public Instrument {
	float amp,*amptable,tabs[2],*in;
	int skip;
	int amp_count;
	float inskip,outskip,dur,inchan;  // Put here so can be rtupdatable

public:
	MIXN();
	virtual ~MIXN();
	int init(float*, short);
	int run();
	};
