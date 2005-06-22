/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _OEQUALIZER_H_
#define _OEQUALIZER_H_ 1

// Biquad equalizer class, based on code by Tom St Denis
// <tomstdenis.home.dhs.org>, which in turn is based on the Cookbook
// formulas for audio EQ biquad filter coefficients by Robert
// Bristow-Johnson. (Google for "Audio-EQ-Cookbook".)
//
// Reimplemented by John Gibson.

#if defined(i386)
 #define ANTI_DENORM
#endif

typedef enum {
   OeqLowPass = 0,
   OeqHighPass,
   OeqBandPassCSG,    // CSG: constant skirt gain; peak gain = Q
   OeqBandPassCPG,    // CPG: constant 0 dB peak gain
   OeqBandPass = OeqBandPassCPG,
   OeqNotch,
   OeqAllPass,
   OeqPeaking,
   OeqLowShelf,
   OeqHighShelf,
   OeqInvalid
} OeqType;

class Oequalizer
{
public:
	Oequalizer(float srate, OeqType type);
   void settype(OeqType type) { _type = type; }
	void setparams(float freq, float Q, float gain = 0.0f);
	inline void clear() { _x1 = _x2 = _y1 = _y2 = 0.0f; }

	inline float next(float input);
	float last() const { return _y1; }

private:
	float _sr;
	OeqType _type;
	float _c0, _c1, _c2, _c3, _c4;
	float _x1, _x2, _y1, _y2;
#ifdef ANTI_DENORM
	float _antidenorm_offset;
#endif
};


inline float Oequalizer::next(float input)
{
	float y0 = (_c0 * input) + (_c1 * _x1) + (_c2 * _x2)
									 - (_c3 * _y1) - (_c4 * _y2);
#ifdef ANTI_DENORM
	y0 += _antidenorm_offset;
	_antidenorm_offset = -_antidenorm_offset;
#endif
	_x2 = _x1;
	_x1 = input;
	_y2 = _y1;
	_y1 = y0;

	return y0;
}

#endif // _OEQUALIZER_H_
