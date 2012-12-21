#include <Instrument.h>
#include <vector>

class CHAIN : public Instrument {
	std::vector<Instrument *>	mInstVector;

public:
	CHAIN();
	virtual ~CHAIN();
	virtual int setup(PFieldSet *);
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
};

