/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// RTcmixDisplay.h - Abstract base class for system-dependent parameter display.
// by John Gibson, based on control/mouse code.
#ifndef _RTCMIXDISPLAY_H_
#define _RTCMIXDISPLAY_H_

#include <stdlib.h>
#include <pthread.h>
#include <labels.h>

class RTcmixDisplay {
public:
	RTcmixDisplay();
	virtual ~RTcmixDisplay();

	// Show the display window on the screen.
	virtual int show() = 0;

	// Labels have the format: "prefix: value units", where value is a formatted
	// double and units is optional.  Example: "cutoff: 2000.0 Hz"
	//
	// Copy the <prefix> and <units> strings into the RTcmixDisplay subclass,
	// and return an id for the caller to use when reporting its current values
	// during run.  If there is no more label space, return -1.  If caller
	// doesn't want a units string, pass NULL for <units>.  The <precision>
	// argument gives the number of digits after the decimal point to display.
	int configureLabel(const char *prefix, const char *units,
                                                   const int precision);

	// Update the value used in the label.
	void updateLabelValue(const int id, const double value);

	// to be called only by createDisplayWindow
	int spawnEventLoop();

protected:
	virtual void doConfigureLabel(const int id, const char *prefix,
                                 const char *units, const int precision) = 0;
	virtual void doUpdateLabelValue(const int id, const double value) = 0;
	virtual bool handleEvents() = 0;

	int _labelCount;
	char *_label[kNumLabels];
	char *_prefix[kNumLabels];
	char *_units[kNumLabels];
	int _precision[kNumLabels];

private:
	inline unsigned long getSleepTime() { return _sleeptime; }
	inline bool runThread() { return _runThread; }
	static void *_eventLoop(void *);

	double _last[kNumLabels];
	unsigned long _sleeptime;
	pthread_t _eventthread;
	bool _runThread;
	int _throttleCount[kNumLabels];
	int _throttle;
};

RTcmixDisplay *createDisplayWindow();

#endif // _RTCMIXDISPLAY_H_
