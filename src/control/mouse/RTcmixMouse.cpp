/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <RTcmixMouse.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>


RTcmixMouse::RTcmixMouse()
	: _xlabelCount(0), _ylabelCount(0),
	  _sleeptime(SLEEP_MSEC * 1000)
{
	for (int i = 0; i < NLABELS; i++) {
		_xprefix[i] = NULL;
		_yprefix[i] = NULL;
		_xunits[i] = NULL;
		_yunits[i] = NULL;
		_xlabel[i] = NULL;
		_ylabel[i] = NULL;
	}
}

RTcmixMouse::~RTcmixMouse()
{
	int retcode = pthread_join(_eventthread, NULL);
	for (int i = 0; i < NLABELS; i++) {
		delete _xprefix[i];
		delete _yprefix[i];
		delete _xunits[i];
		delete _yunits[i];
		delete _xlabel[i];
		delete _ylabel[i];
	}
}

int RTcmixMouse::configureXLabel(const char *prefix, const char *units)
{
	return configureXLabel(prefix, units, DEFAULT_PRECISION);
}

int RTcmixMouse::configureXLabel(const char *prefix, const char *units,
		const int prec)
{
	assert(prefix != NULL);
	if (_xlabelCount == NLABELS)
		return -1;
	const int id = _xlabelCount;
	_xlabelCount++;
	_xprefix[id] = strdup(prefix);
	if (units)
		_xunits[id] = strdup(units);
	_xlabel[id] = new char [LABEL_LENGTH];
	_xlabel[id][0] = 0;
	_xprecision[id] = prec;
	return id;
}

int RTcmixMouse::configureYLabel(const char *prefix, const char *units)
{
	return configureYLabel(prefix, units, DEFAULT_PRECISION);
}

int RTcmixMouse::configureYLabel(const char *prefix, const char *units,
		const int prec)
{
	assert(prefix != NULL);
	if (_ylabelCount == NLABELS)
		return -1;
	const int id = _ylabelCount;
	_ylabelCount++;
	_yprefix[id] = strdup(prefix);
	if (units)
		_yunits[id] = strdup(units);
	_ylabel[id] = new char [LABEL_LENGTH];
	_ylabel[id][0] = 0;
	_yprecision[id] = prec;
	return id;
}

void RTcmixMouse::updateXLabelValue(const int id, const double value)
{
	if (id < 0)
		return;
	assert(id < _xlabelCount);
	const char *units = _xunits[id] ? _xunits[id] : "";
	snprintf(_xlabel[id], LABEL_LENGTH, "%s: %.*f %s",
				_xprefix[id], _xprecision[id], value, units);
	drawXLabels();
}

void RTcmixMouse::updateYLabelValue(const int id, const double value)
{
	if (id < 0)
		return;
	assert(id < _ylabelCount);
	const char *units = _yunits[id] ? _yunits[id] : "";
	snprintf(_ylabel[id], LABEL_LENGTH, "%s: %.*f %s",
				_yprefix[id], _yprecision[id], value, units);
	drawYLabels();
}

void *RTcmixMouse::_eventLoop(void *context)
{
	RTcmixMouse *obj = (RTcmixMouse *) context;
	while (1) {
		if (obj->handleEvents() == false)
			break;
		usleep(obj->getSleepTime());
	}
	return NULL;
}

int RTcmixMouse::spawnEventLoop()
{
	int retcode = pthread_create(&_eventthread, NULL, _eventLoop, this);
	if (retcode != 0)
		fprintf(stderr, "Error creating mouse window thread (%d).\n", retcode);
	return retcode;
}


#ifdef MACOSX
	#include <OSXMouse.h>
#else
	#include <XMouse.h>
#endif

RTcmixMouse *createMouseWindow()
{
#ifdef MACOSX
	RTcmixMouse *mousewin = new OSXMouse();
#else
	RTcmixMouse *mousewin = new XMouse();
#endif
	if (mousewin->spawnEventLoop() != 0) {
		delete mousewin;
		mousewin = NULL;
	}

	return mousewin;
}

