#include <Instrument.h>
#include <math.h>

class Oequalizer;
class Obalance;

class VOCODE3 : public Instrument {
	int _nargs, _branch, _numfilts, _hold;
	int *_maptable;
	float _modtransp, _cartransp, _modq, _carq, _responsetime;
	float _amp, _pan, _nyquist;
	float *_in, *_lastmod;
	double *_modtable_src, *_cartable_src, *_modtable_prev, *_cartable_prev;
	double *_maptable_src, *_scaletable;
	Oequalizer **_modulator_filt, **_carrier_filt;
	Obalance **_balancer;

	int usage();
	inline float convertSmooth(const float smooth);
	inline float updateFreq(float freq, float transp);
	void doupdate();
public:
	VOCODE3();
	virtual ~VOCODE3();
	virtual int init(double p[], int n_args);
	virtual int configure();
	virtual int run();
};


// Any freq < 15 is assumed to be in linear octave format.  Transposition
// is in linear octaves only.

const float kMinCenterFreq = 20.0f;

inline float VOCODE3::updateFreq(float freq, float transp)
{
	if (transp != 0.0f) {
		if (freq >= 15.0f)
			freq = octcps(freq);
		freq = cpsoct(freq + transp);
	}
	else if (freq < 15.0f)
		freq = cpsoct(freq);
	if (freq < kMinCenterFreq)
		freq = kMinCenterFreq;
	if (freq > _nyquist)
		freq = _nyquist;
	return freq;
}

