#include <Instrument.h>

class Ooscili;
class Orand;

class BweNoise {
public:
	BweNoise();
	~BweNoise();
	float next();
private:
	Orand *_rand;
	float _xv[4], _yv[4];
};

class BWESINE : public Instrument {

public:
	BWESINE();
	virtual ~BWESINE();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	BweNoise *_noi;
	Ooscili *_osc;
	int _branch;
	float _amp, _pan;
	double _bandwidth;
};

