/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// RTcmixMouse.h - Abstract base class for system-dependent mouse input.
// by John Gibson, based on Doug Scott's audio device code design.
#ifndef _RTCMIXMOUSE_H_
#define _RTCMIXMOUSE_H_

#include <stdlib.h>
#include <pthread.h>

#define SLEEP_MSEC			10		// How long to nap between polling of events
#define NLABELS				4		// Number of labels per axis
#define LABEL_LENGTH			32		// Total number of chars in one label
#define DEFAULT_PRECISION	3		// Precision of label double values

class RTcmixMouse {
public:
	RTcmixMouse();
	virtual ~RTcmixMouse();

	// Deliver coordinate(s) in range [0, 1].  If coord. is negative, it means
	// that the coord. is invalid, and the caller should use a default instead.
	virtual double getPositionX() const = 0;
	virtual double getPositionY() const = 0;

	// Clients can have a label printed when their value changes.  Labels have
	// the format: "prefix: value units", where value is a formatted double and
	// units is optional.  Example: "cutoff: 2000.0 Hz"
	//
	// Copy the <prefix> and <units> strings into the RTcmixMouse object, and 
	// return an id for the caller to use when reporting its current values
	// during run.  If there is no more label space, return -1.  If caller
	// doesn't want a units string, pass NULL for <units>.  The optional <prec>
	// argument gives the number of digits after the decimal point to display.
	int configureXLabel(const char *prefix, const char *units);
	int configureXLabel(const char *prefix, const char *units, const int prec);
	int configureYLabel(const char *prefix, const char *units);
	int configureYLabel(const char *prefix, const char *units, const int prec);

	// Update the value used in the label.  Note: only the client knows how the
	// mouse coords, given to it in range [0,1], will be scaled.
	void updateXLabelValue(const int id, const double value);
	void updateYLabelValue(const int id, const double value);


	// to be called only by createMouseWindow
	int spawnEventLoop();

protected:
	virtual bool handleEvents() = 0;
	virtual void drawXLabels() = 0;
	virtual void drawYLabels() = 0;

	char *_xlabel[NLABELS];
	char *_ylabel[NLABELS];
	int _xlabelCount;
	int _ylabelCount;

private:
	inline unsigned long getSleepTime() { return _sleeptime; }
	static void *_eventLoop(void *);

	char *_xprefix[NLABELS];
	char *_yprefix[NLABELS];
	char *_xunits[NLABELS];
	char *_yunits[NLABELS];
	int _xprecision[NLABELS];
	int _yprecision[NLABELS];
	double _lastx[NLABELS];
	double _lasty[NLABELS];
	unsigned long _sleeptime;
	pthread_t _eventthread;
};

RTcmixMouse *createMouseWindow();

#endif // _RTCMIXMOUSE_H_
