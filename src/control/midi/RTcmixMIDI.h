/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// RTcmixMIDI.h - Wrapper class for portmidi library, by John Gibson
#ifndef _RTCMIXMIDI_H_
#define _RTCMIXMIDI_H_

#include <portmidi.h>
#include <porttime.h>
#include <pmutil.h>

#define SLEEP_MSEC			1		// How long to nap between polling of events

class RTcmixMIDI {
public:
	RTcmixMIDI();
	virtual ~RTcmixMIDI();

protected:
	virtual bool handleEvents();

private:
	inline bool active() { return _active; }
	inline bool active(bool state) { _active = state; return _active; }
	inline PmQueue *mainToMIDI() { return _mainToMIDI; }
	inline PmQueue *MIDIToMain() { return _MIDIToMain; }
	inline PmStream *instream() { return _instream; }
	inline PmStream *outstream() { return _outstream; }
	static void _processMIDI(PtTimestamp timestamp, void *context);

	PmStream *_instream;
	PmStream *_outstream;
	PmQueue *_mainToMIDI;
	PmQueue *_MIDIToMain;
	bool _active;
};

RTcmixMIDI *createMIDIPort();

#endif // _RTCMIXMIDI_H_
