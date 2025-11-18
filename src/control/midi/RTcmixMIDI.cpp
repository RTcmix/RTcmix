/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include "RTcmixMIDI.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <algorithm>
#include <RTOption.h>
#include <RTcmix.h>
#include <ugens.h>

#define MIDI_TIMER_LOOP 0   /* for future use */

#define DEBUG 0

#if DEBUG
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

// if INBUF_SIZE is 0, PortMidi uses a default value
#define INBUF_SIZE		0
#define MSG_QUEUE_SIZE	32		// NB: sets number of notes queued for scheduler

typedef enum {
	kQuitMsg = 0,
	kNoteOnMsg,
	kNoteOffMsg
} RTMQMessageType;

typedef struct {
	RTMQMessageType type;
	int data0;
	int data1;
	int data2;
} RTMQMessage;


RTcmixMIDIInput::RTcmixMIDIInput()
	: _instream(NULL), _active(false)
{
	clear();
}

RTcmixMIDIInput::~RTcmixMIDIInput()
{
	if (_mainToMIDI) {
		RTMQMessage msg;
		msg.type = kQuitMsg;
		Pm_Enqueue(_mainToMIDI, &msg);

		// wait for acknowlegement
		int spin;
		do {
			spin = Pm_Dequeue(_MIDIToMain, &msg);
		} while (spin == 0);
	}
	Pt_Stop();	// stop timer
	if (_instream)
		Pm_Close(_instream);

	Pm_QueueDestroy(_mainToMIDI);
	Pm_QueueDestroy(_MIDIToMain);

	Pm_Terminate();
}

RTcmixMIDIInput *createMIDIInputPort()
{
	RTcmixMIDIInput *midiport = NULL;
    try {
        midiport = new RTcmixMIDIInput();
    }
    catch (...) {
        return NULL;
    }
    if (midiport->init() == -1) {
        delete midiport;
        midiport = NULL;
    }
	return midiport;
}

int RTcmixMIDIInput::init() {
    _MIDIToMain = Pm_QueueCreate(MSG_QUEUE_SIZE, sizeof(RTMQMessage));
    _mainToMIDI = Pm_QueueCreate(MSG_QUEUE_SIZE, sizeof(RTMQMessage));
    if (_mainToMIDI == NULL || _MIDIToMain == NULL) {
        rterror("setup_midi", "Could not create MIDI message queues.");
        return -1;
    }

    // Start the timer before starting MIDI I/O.  The timer runs the callback
    // function every SLEEP_MSEC milliseconds.
    Pt_Start(SLEEP_MSEC, &_processMIDI, this);

    Pm_Initialize();

    int id = 0;
    const char *devname = RTOption::midiInDevice();
    PRINT("Requested MIDI input device: \"%s\"\n", devname);
    bool devProvided = false, devFound = false;
    if (strlen(devname)) {
        bool found = false;
        const int numdev = Pm_CountDevices();
        devProvided = true;
        for (; id < numdev; id++) {
            const PmDeviceInfo *info = Pm_GetDeviceInfo(id);
            if (info->input) {
                PRINT("Found MIDI input device: \"%s\"\n", info->name);
                if (strcmp(info->name, devname) == 0) {
                    devFound = found = true;
                    break;
                }
            }
        }
        if (!found) {
            id = Pm_GetDefaultInputDeviceID();
        }
    } else {
        id = Pm_GetDefaultInputDeviceID();
    }
    const PmDeviceInfo *info = Pm_GetDeviceInfo(id);
    if (info == NULL) {
        rterror("setup_midi", "Could not open MIDI input device %d.", id);
        return -1;
    }
    else if (!devProvided) {
        rtcmix_warn("setup_midi", "Using default MIDI input device \"%s\".\n", info->name);
    }
    else if (!devFound) {
        rtcmix_warn("setup_midi", "No match for MIDI input device \"%s\""
                        " ... using default \"%s\".", devname, info->name);
    }
	PmError err = Pm_OpenInput(&_instream, id, NULL, INBUF_SIZE, NULL, NULL);
	if (err != pmNoError) {
        rterror("setup_midi", "Could not open MIDI input stream: %s.",
									Pm_GetErrorText(err));
		return -1;
	}

	Pm_SetFilter(_instream, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX);

	// Empty buffer after setting filter, in case anything got through before.
	PmEvent buffer;
	while (Pm_Poll(_instream))
		Pm_Read(_instream, &buffer, 1);

	// enable processing in MIDI thread (_processMIDI)
	active(true);

	return 0;
}

void RTcmixMIDIInput::clear()
{
	for (int chan = 0; chan < 16; chan++) {
		_noteonpitch[chan] = INVALID_MIDIVAL;
		_noteonvel[chan] = INVALID_MIDIVAL;
		_noteoffpitch[chan] = INVALID_MIDIVAL;
		_noteoffvel[chan] = INVALID_MIDIVAL;
		_bend[chan] = INVALID_MIDIVAL;
		_program[chan] = INVALID_MIDIVAL;
		_chanpress[chan] = INVALID_MIDIVAL;
		for (int i = 0; i < 128; i++) {
			_control[chan][i] = INVALID_MIDIVAL;
			_polypress[chan][i] = INVALID_MIDIVAL;
		}
	}
}


// -------------------------------------------------------------------- dump ---
// Just for debugging.

const char *RTcmixMIDIInput::getValueString(const int val)
{
	if (val == INVALID_MIDIVAL)
		return "--";

	static char buf[16];
	snprintf(buf, 16, "%d", val);
	return buf;
}

void RTcmixMIDIInput::dump(const int chan)
{
	printf("\nDumping current MIDI state...\n");
	printf("---------------------------------------- Channel %d\n", chan + 1);
	printf("   Bend:\t%s\n", getValueString(_bend[chan]));
	printf("   Program:\t%s\n", getValueString(_program[chan]));
	printf("   ChanPress:\t%s\n", getValueString(_chanpress[chan]));
	printf("   NoteOnPitch:\t%s\n", getValueString(_noteonpitch[chan]));
	printf("   NoteOnVel:\t%s\n", getValueString(_noteonvel[chan]));
	printf("   NoteOffPitch:\t%s\n", getValueString(_noteoffpitch[chan]));
	printf("   NoteOffVel:\t%s\n", getValueString(_noteoffvel[chan]));
#if 1
	printf("   Control:\n");
	for (int i = 0; i < 128; i++)
		printf("      [%d]:\t%s\n", i, getValueString(_control[chan][i]));
#endif
#if 0
	printf("   PolyPress:\n");
	for (int i = 0; i < 128; i++)
		printf("      [%d]:\t%s\n", i, getValueString(_polypress[chan][i]));
#endif
	printf("\n");
	fflush(stdout);
}


// ------------------------------------------------------------ _processMIDI ---

enum StatusByte {
	kNoteOff = 0x80,
	kNoteOn = 0x90,
	kPolyPress = 0xA0,
	kControl = 0xB0,
	kProgram = 0xC0,
	kChanPress = 0xD0,
	kPitchBend = 0xE0,
	kSystem = 0xF0
};

// This is called from the MIDI worker thread.  The RTcmixMIDIInput object, held by
// main thread, communicates with this worker thread by passing messages
// back and forth.
void RTcmixMIDIInput::_processMIDI(PtTimestamp timestamp, void *context)
{
	RTcmixMIDIInput *obj = (RTcmixMIDIInput *) context;

	// Check for messages from object.
	PmError result;
	do {
		RTMQMessage msg;
		result = Pm_Dequeue(obj->mainToMIDI(), &msg);
		if (result) {
			if (msg.type == kQuitMsg) {
				// acknowledge receipt of quit message
				Pm_Enqueue(obj->MIDIToMain(), &msg);
				obj->active(false);
				return;
			}
		}
	} while (result);

	// Don't poll MIDI until initialization completes.  We still listen for
	// object messages, in case RTcmixMIDIInput::init fails and dtor is then called.
	if (!obj->active())
		return;

	// See if there is any MIDI input to process.
	do {
		result = Pm_Poll(obj->instream());
		if (result) {
			PmEvent buffer;

			if (Pm_Read(obj->instream(), &buffer, 1) == pmBufferOverflow)
				continue;

			// Unless there was overflow, we should have a message now.
			const long status = Pm_MessageStatus(buffer.message);
			const int data1 = Pm_MessageData1(buffer.message);
			const int data2 = Pm_MessageData2(buffer.message);

			const int chan = status & 0x0F;

			switch (status & 0xF0) {
				case kNoteOn:
					if (data2 > 0) {
						obj->setNoteOnPitch(chan, data1);
						obj->setNoteOnVel(chan, data2);
						obj->noteOnTrigger(chan, data1, data2);
					}
					else {	// note on w/ vel=0 is logically a note off
						obj->setNoteOffPitch(chan, data1);
						obj->setNoteOffVel(chan, data2);
						obj->noteOffTrigger(chan, data1, data2);
					}
					break;
				case kNoteOff:
					obj->setNoteOffPitch(chan, data1);
					obj->setNoteOffVel(chan, data2);
					obj->noteOffTrigger(chan, data1, data2);
					break;
				case kPolyPress:
					obj->setPolyPress(chan, data1, data2);
					break;
				case kControl:
					obj->setControl(chan, data1, data2);
					break;
				case kProgram:
					obj->setProgram(chan, data1);
					break;
				case kChanPress:
					obj->setChanPress(chan, data1);
					break;
				case kPitchBend:
					obj->setBend(chan, ((data2 << 7) + data1) - 8192);
					break;
				case kSystem:
				default:
#if DEBUG > 1
					printf("0x%.2x, %ld, %ld\n", (u_char) status, data1, data2);
#endif
					break;
			}
#if DEBUG > 1
			obj->dump(chan);
#endif
		}
	} while (result);
}

// These functions pass a message, from the MIDI worker thread to the main
// thread, containing the data for a new noteon or noteoff event.  If we
// were to act on that data directly here, by calling a scheduler function,
// then the note would be running within the MIDI worker thread!  Not what
// we want.

// FIXME: Presumably the scheduler would call RTcmixMIDIInput::checkMIDITrigger,
// which would dequeue such messages and take appropriate action, starting or
// stopping notes.  Since RTcmixMIDIInput is supposed to be dynamically loaded, how
// would scheduler know if and what to call?

// Answer: We need to create a new scheduler callback and have this code
// register a new callback which checks for pending MIDI events.  This layer would then
// need to call some new code on the scheduler which knew how to map incoming MIDI
// channels and program numbers to specific instruments.

void RTcmixMIDIInput::noteOnTrigger(int chan, int pitch, int velocity)
{
	RTMQMessage msg;
	msg.type = kNoteOnMsg;
	msg.data0 = chan;
	msg.data1 = pitch;
	msg.data2 = velocity;
	Pm_Enqueue(_MIDIToMain, &msg);
}

void RTcmixMIDIInput::noteOffTrigger(int chan, int pitch, int velocity)
{
	RTMQMessage msg;
	msg.type = kNoteOffMsg;
	msg.data0 = chan;
	msg.data1 = pitch;
	msg.data2 = velocity;
	Pm_Enqueue(_MIDIToMain, &msg);
}


//************************************************//

static void startCallback(void *context)
{
    RTcmixMIDIOutput *midiport = (RTcmixMIDIOutput*) context;
    // We set the MIDI latency to one buffer's length times the buffer count.
    long latency = long((RTcmix::bufsamps() * RTOption::bufferCount()) * 1000.0 / RTcmix::sr());
    PRINT("RTcmixMIDIOutput startCallback: latency = %ld ms\n", latency);
    midiport->start(latency);
    midiport->sendMIDIStart(0);
}

static void stopCallback(void *context)
{
    PRINT("RTcmixMIDIOutput stop callback called\n");
    RTcmixMIDIOutput *midiout = (RTcmixMIDIOutput*) context;
    if (!midiout->isStopped()) {
        midiout->sendMIDIStop(0);
        usleep(100000);     // wait for messages!
        midiout->stop();
    }
}

void RTcmixMIDIOutput::_midiCallback(PtTimestamp timestamp, void *context)
{

}

RTcmixMIDIOutput::RTcmixMIDIOutput(const char *portname) : _portname(portname), _deviceID(0), _outstream(NULL)
{
    
}

RTcmixMIDIOutput::~RTcmixMIDIOutput()
{
    PRINT("Destroying RTcmixMIDIOutput instance\n");
    RTcmix::unregisterAudioStartCallback(startCallback, this);
    RTcmix::unregisterAudioStopCallback(stopCallback, this);
    stop();
#if MIDI_TIMER_LOOP
    Pt_Stop();
#endif
    Pm_Terminate();
}

int RTcmixMIDIOutput::init()
{
    Pm_Initialize();
    
    const char *devname = _portname ? _portname : RTOption::midiOutDevice();
    PRINT("Requested MIDI output device: \"%s\"\n", devname);
    if (strlen(devname)) {
        bool found = false;
        const int numdev = Pm_CountDevices();
        for (int id=0; id < numdev; id++) {
            const PmDeviceInfo *info = Pm_GetDeviceInfo(id);
            if (info->output) {
                PRINT("Found MIDI output device: \"%s\"\n", info->name);
                if (strcmp(info->name, devname) == 0) {
                    _deviceID = id;
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            rtcmix_warn("setup_midi", "No match for MIDI output device \"%s\""
                    " ... using default.", devname);
            _deviceID = Pm_GetDefaultOutputDeviceID();
        }
    }
    else {
        rtcmix_advise("setup_midi", "Using default MIDI output device.");
        _deviceID = Pm_GetDefaultOutputDeviceID();
    }
    const PmDeviceInfo *info = Pm_GetDeviceInfo(_deviceID);
    if (info == NULL) {
        rterror("setup_midi", "Could not open MIDI output device %d.", _deviceID);
        return -1;
    }
#if MIDI_TIMER_LOOP
    Pt_Start(SLEEP_MSEC, &_midiCallback, this);
#endif
    return 0;
}

int RTcmixMIDIOutput::start(long latency)
{
    PRINT("Opening Portmidi output stream with latency of %ld ms\n", latency);
    PmError err = Pm_OpenOutput(&_outstream, _deviceID, NULL, INBUF_SIZE, NULL, NULL, latency);
    if (err != pmNoError) {
        rterror(NULL,"Could not open MIDI output stream: %s.", Pm_GetErrorText(err));
        return -1;
    }
    return 0;
}

int RTcmixMIDIOutput::stop()
{
    PRINT("Closing Portmidi output stream\n");
    if (_outstream) {
        Pm_Close(_outstream);
        _outstream = NULL;
    }
    return 0;
}

inline uchar make_status(uchar type, uchar chan) { return chan | type; }

void RTcmixMIDIOutput::sendNoteOn(long timestamp, uchar chan, uchar pitch, uchar vel)
{
    PmEvent buffer;
    buffer.message = Pm_Message(make_status(kNoteOn, chan), pitch, vel);
    buffer.timestamp = timestamp;
    PRINT("RTcmixMIDIOutput::sendNoteOn: sending event with ts = %ld\n", timestamp);
    AutoLock al(this);
    _noteList.push_front(MidiItem(chan, pitch));
    Pm_Write(outstream(), &buffer, 1);
}

void RTcmixMIDIOutput::sendNoteOff(long timestamp, uchar chan, uchar pitch, uchar vel)
{
    PmEvent buffer;
    buffer.message = Pm_Message(make_status(kNoteOff, chan), pitch, vel);
    buffer.timestamp = timestamp;
    PRINT("RTcmixMIDIOutput::sendNoteOff: sending event with ts = %ld\n", timestamp);
    AutoLock al(this);
    // The following prevents us from sending a note-off when there are no active notes
    std::list<MidiItem>::iterator ni = std::find(_noteList.begin(), _noteList.end(), MidiItem(chan, pitch));
    if (ni != _noteList.end()) {
        _noteList.erase(ni);
        Pm_Write(outstream(), &buffer, 1);
    }
}

void RTcmixMIDIOutput::sendControl(long timestamp, uchar chan, uchar control, unsigned value)
{
    PmEvent buffer;
    buffer.message = Pm_Message(make_status(kControl, chan), control, value);
    buffer.timestamp = timestamp;
    PRINT("RTcmixMIDIOutput::sendControl: sending event %d value %d on chan %d with ts = %ld\n", control, value, chan, timestamp);
    AutoLock al(this);
    Pm_Write(outstream(), &buffer, 1);
}

void RTcmixMIDIOutput::sendPitchBend(long timestamp, uchar chan, unsigned value)
{
    PmEvent buffer;
    uchar msb = (value >> 8) & 0xff;
    uchar lsb = value & 0xff;
    buffer.message = Pm_Message(make_status(kPitchBend, chan), lsb, msb);
    buffer.timestamp = timestamp;
    PRINT("RTcmixMIDIOutput::sendPitchBend: sending event with ts = %ld\n", timestamp);
    AutoLock al(this);
    Pm_Write(outstream(), &buffer, 1);
}

void RTcmixMIDIOutput::sendProgramChange(long timestamp, uchar chan, uchar program)
{
    PmEvent buffer;
    buffer.message = Pm_Message(make_status(kProgram, chan), program, 0);
    buffer.timestamp = timestamp;
    PRINT("RTcmixMIDIOutput::sendProgramChange: sending event with ts = %ld\n", timestamp);
    AutoLock al(this);
    Pm_Write(outstream(), &buffer, 1);
}

void RTcmixMIDIOutput::sendSysEx(long timestamp, unsigned char *msg)
{
    AutoLock al(this);
    Pm_WriteSysEx(outstream(), timestamp, msg);
}

void RTcmixMIDIOutput::sendMIDIStart(long timestamp) {
    PmEvent startEvent, SPPEvent;
//    startEvent.message = Pm_Message(0xFA, 0, 0);
//    startEvent.timestamp = timestamp;
    SPPEvent.message = Pm_Message(0xF2, 0, 0);
    SPPEvent.timestamp = timestamp;
    PmEvent buffers[16];
    for (int chan = 0; chan < 16; ++chan) {
        buffers[chan].message = Pm_Message(make_status(kControl, chan), 121, 0);   // ResetAllControllers
        buffers[chan].timestamp = timestamp;
    }
//    PRINT("RTcmixMIDIOutput::sendMIDIStart: sending MIDI ResetAllControllers, Start and SPP events with ts = %ld\n", timestamp);
//    PRINT("RTcmixMIDIOutput::sendMIDIStart: sending MIDI ResetAllControllers and MTC at ts = %ld\n", timestamp);
    AutoLock al(this);
    Pm_Write(outstream(), buffers, 16);
//    Pm_Write(outstream(), &SPPEvent, 1);
    // MMC MIDI sysex for "go to beginning of track"
//    unsigned char msg[14] = { 0xF0, 0x7F, 0x7F, 0x06, 0x44, 0x06, 0x01, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 };
//    Pm_WriteSysEx(outstream(), timestamp, msg);
    if (RTOption::sendMIDIRecordAutoStart()) {
        PRINT("RTcmixMIDIOutput::sendMIDIStart: sending MIDI ResetAllControllers and CC 119 ts = %ld\n", timestamp);
        startEvent.message = Pm_Message(make_status(kControl, 0), 119, 127);
        Pm_Write(outstream(), &startEvent, 1);
    }

    PRINT("RTcmixMIDIOutput::sendMIDIStart: Done\n");
}

#define SEND_MIDI_NOTE_OFFS

void RTcmixMIDIOutput::sendMIDIStop(long timestamp)
{
#ifdef SEND_MIDI_NOTE_OFFS
    rtcmix_debug(NULL, "Sending MIDI Note Off events for Remaining Notes");
    AutoLock al(this);
    for (std::list<MidiItem>::iterator it = _noteList.begin(); it != _noteList.end(); ++it){
        MidiItem item = *it;
        PmEvent buffer;
        buffer.message = Pm_Message(make_status(kNoteOff, item.first), item.second, 0);
        buffer.timestamp = timestamp;
        Pm_Write(outstream(), &buffer, 1);
    }
#else
    PmEvent buffer1;
    buffer1.message = Pm_Message(0xFF, 0, 0);   // system reset
    buffer1.timestamp = timestamp;
    PmEvent buffers[16];
    rtcmix_debug(NULL, "Sending MIDI System Reset, Stop, and All Sound Off events");
    for (int chan = 0; chan < 16; ++chan) {
        buffers[chan].message = Pm_Message(make_status(kControl, chan), 120, 0);   // AllSoundOff
        buffers[chan].timestamp = timestamp;
    }
    PRINT("RTcmixMIDIOutput::sendMIDIStop: sending System Reset and AllSoundOff events with ts = %ld\n", timestamp);
    AutoLock al(this);
    Pm_Write(outstream(), buffers, 16);
    Pm_Write(outstream(), &buffer1, 1);
#endif
//    PRINT("RTcmixMIDIOutput::sendMIDIStop: sending MIDI Stop with ts = %ld\n", timestamp);
    if (RTOption::sendMIDIRecordAutoStart()) {
        PRINT("RTcmixMIDIOutput::sendMIDIStop: sending CC 119 ts = %ld\n", timestamp);
        PmEvent stopEvent;
//      stopEvent.message = Pm_Message(0xFC, 0, 0);   // timecode stop
        stopEvent.message = Pm_Message(make_status(kControl, 0), 119, 0);
        stopEvent.timestamp = timestamp;
        Pm_Write(outstream(), &stopEvent, 1);
    }
    PRINT("RTcmixMIDIOutput::sendMIDIStop: Done\n");
}

RTcmixMIDIOutput *createMIDIOutputPort(const char *portname)
{
    // We need rtsetparams in order to set the latency of the MIDI system.
    if (!RTcmix::rtsetparams_was_called()) {
        rterror(NULL, "You must call rtsetparams before setting up MIDI output.");
        return NULL;
    }
    RTcmixMIDIOutput *midiport = NULL;
    try {
        midiport = new RTcmixMIDIOutput(portname);
    }
    catch (...) {
        return NULL;
    }
    if (midiport->init() == -1) {
        delete midiport;
        return NULL;
    }

    RTcmix::registerAudioStartCallback(startCallback, midiport);
    RTcmix::registerAudioStopCallback(stopCallback, midiport);
    
    return midiport;
}
