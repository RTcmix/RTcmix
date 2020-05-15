#include <Instrument.h>
#include <random>

class DUST : public Instrument {

public:
	DUST();
	virtual ~DUST();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	int _nargs, _branch;
	float _amp, _pan, _density, _range;
	Orand *_dice;
	std::mt19937 _randgen;
	std::uniform_real_distribution<double> _dist;
};
