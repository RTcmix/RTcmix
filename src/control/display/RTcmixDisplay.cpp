/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <RTcmixDisplay.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>


RTcmixDisplay::RTcmixDisplay()
	: _labelCount(0), _sleeptime(SLEEP_MSEC * 1000)
{
	for (int i = 0; i < NLABELS; i++) {
		_prefix[i] = NULL;
		_units[i] = NULL;
		_label[i] = NULL;
		_last[i] = -1.0;
	}
}

RTcmixDisplay::~RTcmixDisplay()
{
	if (_eventthread) {
		_runThread = false;
		pthread_join(_eventthread, NULL);
	}
	for (int i = 0; i < NLABELS; i++) {
		delete _prefix[i];
		delete _units[i];
		delete _label[i];
	}
}

int RTcmixDisplay::configureLabel(const char *prefix, const char *units,
		const int precision)
{
	assert(prefix != NULL);
	if (_labelCount == NLABELS)
		return -1;
	const int id = _labelCount;
	_labelCount++;

	doConfigureLabel(id, prefix, units, precision);

	return id;
}

void RTcmixDisplay::updateLabelValue(const int id, const double value)
{
	if (id < 0)							// this means user ran out of labels
		return;
	assert(id < _labelCount);
	if (value == _last[id])
		return;

	doUpdateLabelValue(id, value);

	_last[id] = value;
}

void *RTcmixDisplay::_eventLoop(void *context)
{
	RTcmixDisplay *obj = (RTcmixDisplay *) context;
	while (obj->runThread()) {
		if (obj->handleEvents() == false)
			break;
		usleep(obj->getSleepTime());
	}
	return NULL;
}

int RTcmixDisplay::spawnEventLoop()
{
	_runThread = true;
	int retcode = pthread_create(&_eventthread, NULL, _eventLoop, this);
	if (retcode != 0)
		fprintf(stderr, "Error creating display window thread (%d).\n", retcode);
	return retcode;
}


#ifdef MACOSX
	#include <OSXDisplay.h>
#else
	#include <XDisplay.h>
#endif

RTcmixDisplay *createDisplayWindow()
{
#ifdef MACOSX
	RTcmixDisplay *displaywin = new OSXDisplay();
#else
	RTcmixDisplay *displaywin = new XDisplay();
#endif
	if (displaywin->show() != 0 || displaywin->spawnEventLoop() != 0) {
		delete displaywin;
		displaywin = NULL;
	}

	return displaywin;
}

