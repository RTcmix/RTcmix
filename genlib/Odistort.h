/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

// Class for distorting input, based on the algorithms provided with STRUM
// (original code by Charlie Sullivan), plus other code found in the musicdsp
// archives (musicdsp.org).
//
// To use Odistort, do something like this...
//
//    // Pass name of static distortion method, in this case "SoftClip".
//    Odistort *dist = new Odistort(Odistort::SoftClip);
//
//    // for each sample...
//    float output = dist->next(input);
//
// Note that most of these functions assume the input is in range [-1,1].
//
// Also, some algorithms make use of an extra parameter, which must be passed
// in via the next method.  These are identified below.  If you're not using
// one of these functions, then omit the second parameter.
//                                                            -JGG, 7/10/05

#ifndef _ODISTORT_H_
#define _ODISTORT_H_ 1

class Odistort {

public:
	typedef float (*DistortFunction)(float input, float data);

	Odistort(DistortFunction fun = Odistort::SoftClip);
	~Odistort();

	// Here are the available DistortFunction's...

	// From STRUM/dist.c, used in all the elec. guitar feedback variants.
	// Assumes input in [-1,1].  <data> ignored.
	static float SoftClip(float input, float data);

	// From STRUM/dist.c, commented out there.
	// Assumes input in [-1,1].  <data> ignored.
	// FIXME: not sure this is right  -JGG
	static float SimpleTube(float input, float data);

	// Contributed to musicdsp archives by Laurent de Soras.
	// <data> is the clipping hardness.  1: smooth clipping, 100: hard clipping.
	static float VariableClip(float input, float hardness);

	// Contributed to musicdsp archives by Bram de Jong.
	// Assumes input in [-1,1].  <a> controls amount of distortion, and can be
	// from 1 to c. 100: 1 gives slight distortion, larger <a> gives much more.
	static float WaveShape(float input, float a);

	// Pass your own function to be called by next().
	void setDistortFunction(DistortFunction fun) { _fun = fun; }

	inline float next(float input, float data = 0.0f) const;

private:
	DistortFunction _fun;
};


inline float Odistort::next(float input, float data) const
{
	return (*_fun)(input, data);
}

#endif // _ODISTORT_H_
