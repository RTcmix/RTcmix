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
	void clear();
	void dump();

private:
	inline bool active() { return _active; }
	inline bool active(bool state) { _active = state; return _active; }
	inline PmQueue *mainToMIDI() { return _mainToMIDI; }
	inline PmQueue *MIDIToMain() { return _MIDIToMain; }
	inline PmStream *instream() { return _instream; }
	inline PmStream *outstream() { return _outstream; }

	inline int getNoteOnVel(int chan, int pitch)
						{ return _noteonvel[chan][pitch]; }
	inline void setNoteOnVel(int chan, int pitch, int val)
						{ _noteonvel[chan][pitch] = val; }
	inline int getNoteOffVel(int chan, int pitch)
						{ return _noteoffvel[chan][pitch]; }
	inline void setNoteOffVel(int chan, int pitch, int val)
						{ _noteoffvel[chan][pitch] = val; }
	inline int getPolyPress(int chan, int pitch)
						{ return _polypress[chan][pitch]; }
	inline void setPolyPress(int chan, int pitch, int val)
						{ _polypress[chan][pitch] = val; }
	inline int getControl(int chan, int controlnum)
						{ return _control[chan][controlnum]; }
	inline void setControl(int chan, int controlnum, int val)
						{ _control[chan][controlnum] = val; }
	inline int getBend(int chan)
						{ return _bend[chan]; }
	inline void setBend(int chan, int val)
						{ _bend[chan] = val; }
	inline int getProgram(int chan)
						{ return _program[chan]; }
	inline void setProgram(int chan, int val)
						{ _program[chan] = val; }
	inline int getChanPress(int chan)
						{ return _chanpress[chan]; }
	inline void setChanPress(int chan, int val)
						{ _chanpress[chan] = val; }

	static void _processMIDI(PtTimestamp timestamp, void *context);

	PmStream *_instream;
	PmStream *_outstream;
	PmQueue *_mainToMIDI;
	PmQueue *_MIDIToMain;
	bool _active;

	int _bend[16];
	int _program[16];
	int _chanpress[16];
	int _noteonvel[16][128];
	int _noteoffvel[16][128];
	int _control[16][128];
	int _polypress[16][128];
};

RTcmixMIDI *createMIDIPort();

#endif // _RTCMIXMIDI_H_
