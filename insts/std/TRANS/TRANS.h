#include <Instrument.h>
#include <rtdefs.h>

class TRANS : public Instrument {
	float *amptable, tabs[2];
	int incount;
	float increment;
	float counter;
	float skip;
	float amp;
	float old[MAXCHANS], vold[MAXCHANS];
public:
	TRANS();
	int init(float*, short);
	int run();
};
