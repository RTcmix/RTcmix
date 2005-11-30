// RTcmix - Copyright (C) 2005  The RTcmix Development Team
// See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
// the license to this software and for a DISCLAIMER OF ALL WARRANTIES.

#include <RTOscPField.h>
#include <RTcmixOSC.h>
#include <PField.h>
#include <Ougens.h>
#include <string.h>
#include <unistd.h>
#include <float.h>
#include <assert.h>

extern int resetval;		// declared in src/rtcmix/minc_functions.c

const double kInvalidValue = DBL_MAX;

RTOscPField::RTOscPField(
		RTcmixOSC			*oscserver,
		const char 			*path,
		const int			index,
		const double		inputmin,
		const double		inputmax,
		const double		outputmin,
		const double		outputmax,
		const double		defaultval,
		const double		lag)				// in range [0, 1]
	: RTNumberPField(0),
	  _oscserver(oscserver), _index(index),
	  _inputmin(inputmin), _inputmax(inputmax), _outputmin(outputmin),
	  _default(defaultval), _value(kInvalidValue), _callbackReturn(0)
{
	assert(_oscserver != NULL);
	assert(_index >= 0);

	_path = new char [strlen(path) + 1];
	strcpy(_path, path);

	_inputdiff = _inputmax - _inputmin;
	_outputdiff = outputmax - _outputmin;

	// NOTE: We rely on the control rate in effect when this PField is created.
	_filter = new Oonepole(resetval);
	_filter->setlag(lag);

	while (!_oscserver->ready())
		usleep(100);
	_oscserver->registerPField(this, handler);
}

RTOscPField::~RTOscPField()
{
	delete [] _path;
}

double RTOscPField::doubleValue(double) const
{
	// map _value, clamped to input range, into output range
	double val = _value;
	if (val == kInvalidValue)
		val = _default;
	else {
		if (val < _inputmin)
			val = _inputmin;
		else if (val > _inputmax)
			val = _inputmax;
		val = ((val - _inputmin) * _outputdiff / _inputdiff) + _outputmin;
	}
	return _filter->next(val);
}

int RTOscPField::handler(const char *path, const char *types, lo_arg **argv,
		int argc, lo_message msg, void *context)
{
	RTOscPField *pfield = (RTOscPField *) context;
	assert(pfield != NULL);

	const int index = pfield->index();
	if (index < argc) {
		lo_type type = (lo_type) types[index];
		if (type == LO_FLOAT)	// the most common one
			pfield->value(argv[index]->f);
		else if (lo_is_numerical_type(type)) {
			double val = lo_hires_val(type, argv[index]);
			pfield->value(val);
		}
		else
			fprintf(stderr, "WARNING: incoming OSC value of type \'%c\' can't "
								 "be coerced to double.\n", type);
	}
	return pfield->callbackReturn();
}

