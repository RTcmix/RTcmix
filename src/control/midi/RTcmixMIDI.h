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
#include <RTMIDIOutput.h>
#include <Lockable.h>
#include <list>

#define SLEEP_MSEC			1		// How long to nap between polling of events
#define INVALID_MIDIVAL    99999

typedef unsigned char uchar;

class RTcmixMIDIInput {
public:
	RTcmixMIDIInput();
	virtual ~RTcmixMIDIInput();
	int init();
	void clear();
	void dump(int chan);

	// We store the last note on/off pitch and velocity values into an array
	// for reference in a manner similar to that of controllers.  This is *not*
	// for triggering new notes.
	inline int getNoteOnPitch(int chan)
						{ return _noteonpitch[chan]; }
	inline int getNoteOnVel(int chan)
						{ return _noteonvel[chan]; }
	inline int getNoteOffPitch(int chan)
						{ return _noteoffpitch[chan]; }
	inline int getNoteOffVel(int chan)
						{ return _noteoffvel[chan]; }

	inline int getPolyPress(int chan, int pitch)
						{ return _polypress[chan][pitch]; }
	inline int getControl(int chan, int controlnum)
						{ return _control[chan][controlnum]; }
	inline int getBend(int chan)
						{ return _bend[chan]; }
	inline int getProgram(int chan)
						{ return _program[chan]; }
	inline int getChanPress(int chan)
						{ return _chanpress[chan]; }

private:
	inline bool active() { return _active; }
	inline bool active(bool state) { _active = state; return _active; }
	inline PmQueue *mainToMIDI() { return _mainToMIDI; }
	inline PmQueue *MIDIToMain() { return _MIDIToMain; }
	inline PmStream *instream() { return _instream; }

	void noteOnTrigger(int chan, int pitch, int velocity);
	void noteOffTrigger(int chan, int pitch, int velocity);

	inline void setNoteOnPitch(int chan, int val)
						{ _noteonpitch[chan] = val; }
	inline void setNoteOnVel(int chan, int val)
						{ _noteonvel[chan] = val; }
	inline void setNoteOffPitch(int chan, int val)
						{ _noteoffpitch[chan] = val; }
	inline void setNoteOffVel(int chan, int val)
						{ _noteoffvel[chan] = val; }

	inline void setPolyPress(int chan, int pitch, int val)
						{ _polypress[chan][pitch] = val; }
	inline void setControl(int chan, int controlnum, int val)
						{ _control[chan][controlnum] = val; }
	inline void setBend(int chan, int val)
						{ _bend[chan] = val; }
	inline void setProgram(int chan, int val)
						{ _program[chan] = val; }
	inline void setChanPress(int chan, int val)
						{ _chanpress[chan] = val; }
	const char *getValueString(int val);

	static void _processMIDI(PtTimestamp timestamp, void *context);

	PmStream *_instream;
	PmQueue *_mainToMIDI;
	PmQueue *_MIDIToMain;
	bool _active;

	int _noteonpitch[16];
	int _noteonvel[16];
	int _noteoffpitch[16];
	int _noteoffvel[16];
	int _polypress[16][128];
	int _control[16][128];
	int _bend[16];
	int _program[16];
	int _chanpress[16];
};

class RTcmixMIDIOutput : public RTMIDIOutput, private Lockable {
public:
    RTcmixMIDIOutput(const char *portname);
    virtual ~RTcmixMIDIOutput();
    int init();
    
    int start(long latency);
    int stop();
    bool isStopped() { return _outstream == NULL; }
    
    void sendMIDIStart(long timestamp);
    void sendMIDIStop(long timestamp);

    virtual void sendNoteOn(long timestamp, uchar chan, uchar pitch, uchar vel);
    virtual void sendNoteOff(long timestamp, uchar chan, uchar pitch, uchar vel);
    virtual void sendControl(long timestamp, uchar chan, uchar control, unsigned value);
    virtual void sendPitchBend(long timestamp, uchar chan, unsigned value);
    virtual void sendProgramChange(long timestamp, uchar chan, uchar program);
    virtual void sendSysEx(long timestamp, unsigned char *msg);

protected:
    static void _midiCallback(PtTimestamp timestamp, void *context);
    inline PmStream *outstream() { return _outstream; }
private:
    const char *                        _portname;
    int                                 _deviceID;
    PmStream *                          _outstream;
    typedef std::pair<uchar, uchar> MidiItem;
    std::list<MidiItem>                 _noteList;
};

RTcmixMIDIInput *createMIDIInputPort();

RTcmixMIDIOutput *createMIDIOutputPort(const char *portname);

#endif // _RTCMIXMIDI_H_
