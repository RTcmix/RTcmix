#include "Ougens.h"
#include <Instrument.h>

class MYWAVETABLE : public Instrument {
	Ooscili *theOscil;
	Ooscili *theEnv;
        float amp;
        float spread;

public:
	MYWAVETABLE();
	virtual ~MYWAVETABLE();
        int init(double p[], int n_args);
        int run();
	virtual double setfreq(double v);
        };
