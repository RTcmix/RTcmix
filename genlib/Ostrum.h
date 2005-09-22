/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// Plucked string object based on the original cmix strum work by
// Charlie Sullivan.  -JGG, 7/10/05

#ifndef _OSTRUM_H_
#define _OSTRUM_H_ 1

class Ostrum {

public:
	Ostrum(float srate, float freq, int squish, float fundDecayTime = 1.0f,
					float nyquistDecayTime = 0.1f);
	~Ostrum();

	// Change frequency while note is playing.
	inline void setfreq(float freq);

	// Change frequency and decay times while note is playing.
	inline void setfreqdecay(float freq, float fundDecayTime,
	                                     float nyquistDecayTime);

	// Generate a new plucked string sample.  If input is given, it will be
	// mixed in with the plucked string signal and added into the delay line.
	float next(float input = 0.0f);

private:
	static const float kMinFreq;

	void sset(float freq, float fundDecayTime, float nyquistDecayTime);
	void randfill();
	void squisher(int squish);

	float _srate;
	float _maxfreq;
	float _funddcy, _nyqdcy;
	int _dlen;

	int _n, _p;
	float *_d;
	float _a0, _a1, _a2, _a3;
	float _dca0, _dca1, _dcb1, _dcz1;
};


inline void Ostrum::setfreq(float freq)
{
	sset(freq, _funddcy, _nyqdcy);
}

inline void Ostrum::setfreqdecay(float freq, float fundDecayTime,
	float nyquistDecayTime)
{
	_funddcy = fundDecayTime;
	_nyqdcy = nyquistDecayTime;
	sset(freq, _funddcy, _nyqdcy);
}

#endif // _OSTRUM_H_
