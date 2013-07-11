#include "maxdispargs.h"

class MAXMESSAGE : public Instrument {
	float thevals[MAXDISPARGS];
	int nvals;

public:
	MAXMESSAGE();
	virtual ~MAXMESSAGE();
	virtual int init(double*, int);
	virtual int run();
	};
