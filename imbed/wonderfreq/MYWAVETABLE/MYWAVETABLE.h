#include "Ougens.h"
#include <Instrument.h>

class MYWAVETABLE : public Instrument {
	Ooscili *theOscil;
	Ooscili *theEnv;
	float _amp;
	float _pitch;
	float _spread;
	int	  _updateCounter;

public:
	MYWAVETABLE();
	virtual ~MYWAVETABLE();
	virtual int init(double p[], int n_args);
	virtual int run();
};
