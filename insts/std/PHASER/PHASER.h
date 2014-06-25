#include <Instrument.h>      // the base class for this instrument
#include <vector>

using std::vector;

class PHASER : public Instrument {

public:
	PHASER();
	virtual ~PHASER();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	float *_in;
	int _nargs, _inchan, _branch;
	float _amp, _wetdry, _pan, _lfofreq, _reverbtime;
	int _insamps; 
	int _numfilters;

	Ooscili *lfo;
	
	Oallpassi *allpassptr;				// temporary pointer to initialize vector
	vector<Oallpassi*> _filtervector;	// array of allpass filters
};

