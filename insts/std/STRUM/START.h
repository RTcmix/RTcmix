#include "strums.h"

class START : public Instrument {
	float spread;
	strumq *strumq1;
	int deleteflag;

public:
	START();
	virtual ~START();
	int init(float*, short);
	int run();
	};
