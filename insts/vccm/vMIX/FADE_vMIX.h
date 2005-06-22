#include <Instrument.h>

class FADE_vMIX : public Instrument {
	float dur;

public:
	FADE_vMIX();
	virtual ~FADE_vMIX();
	int init(double*, int);
	int run();
	};
