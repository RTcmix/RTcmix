#include "strums.h"

class FRET : public Instrument {
	float freq,tf0,tfN;
	float spread;
	strumq *strumq1;
	int firsttime;

public:
	FRET();
	int init(float*, int);
	int run();
	};
