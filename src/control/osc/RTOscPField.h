// RTcmix - Copyright (C) 2005  The RTcmix Development Team
// See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
// the license to this software and for a DISCLAIMER OF ALL WARRANTIES.

#ifndef _RTOSCPFIELD_H_
#define _RTOSCPFIELD_H_

#include <PField.h>
#include <lo.h>

class Oonepole;
class RTcmixOSC;

class RTOscPField : public RTNumberPField {
public:
	RTOscPField(
		RTcmixOSC			*oscserver,
		const char 			*path,
		const int			index,
		const double		inputmin,
		const double		inputmax,
		const double		outputmin,
		const double		outputmax,
		const double		defaultval,
		const double		lag);				// in range [0, 1]

	virtual double doubleValue(double) const;

	// These functions are called either by this object's static callback
	// function or by the RTcmixOSC object.
	inline void callbackReturn(int val) { _callbackReturn = val; }
	inline int callbackReturn() const { return _callbackReturn; }
	inline int index() const { return _index; }
	inline void value(double value) { _value = value; }
	inline double defaultval() const { return _default; }
	inline char *path() const { return _path; }

	// callback for liblo
	static int handler(const char *path, const char *types, lo_arg **argv,
		int argc, lo_message msg, void *context);

protected:
	virtual ~RTOscPField();

private:
	Oonepole *_filter;
	RTcmixOSC *_oscserver;
	char *_path;
	int _index;
	double _inputmin;
	double _inputmax;
	double _inputdiff;
	double _outputmin;
	double _outputdiff;
	double _default;
	double _value;
	int _callbackReturn;
};

#endif // _RTOSCPFIELD_H_
