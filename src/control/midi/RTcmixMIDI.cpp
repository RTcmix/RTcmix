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
#include <RTOption.h>
#include <RTcmix.h>

#define DEBUG 0

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

int RTcmixMIDIInput::init()
{
	_MIDIToMain = Pm_QueueCreate(MSG_QUEUE_SIZE, sizeof(RTMQMessage));
	_mainToMIDI = Pm_QueueCreate(MSG_QUEUE_SIZE, sizeof(RTMQMessage));
	if (_mainToMIDI == NULL || _MIDIToMain == NULL) {
		fprintf(stderr, "Could not create MIDI message queues.\n");
		return -1;
	}

	// Start the timer before starting MIDI I/O.  The timer runs the callback
	// function every SLEEP_MSEC milliseconds.
	Pt_Start(SLEEP_MSEC, &_processMIDI, this);

	Pm_Initialize();

	int id = 0;
	const char *devname = RTOption::midiInDevice();
#if DEBUG
	printf("Requested MIDI input device: \"%s\"\n", devname);
#endif
	if (strlen(devname)) {
		bool found = false;
		const int numdev = Pm_CountDevices();
		for ( ; id < numdev; id++) {
			const PmDeviceInfo *info = Pm_GetDeviceInfo(id);
			if (info->input) {
#if DEBUG
				printf("Found MIDI input device: \"%s\"\n", info->name);
#endif
				if (strcmp(info->name, devname) == 0) {
					found = true;
					break;
				}
			}
		}
		if (!found) {
			fprintf(stderr, "WARNING: no match for MIDI input device \"%s\""
							" ... using default.\n", devname);
			id = Pm_GetDefaultInputDeviceID();
		}
	}
	else {
		fprintf(stderr, "WARNING: using default MIDI input device.\n");
		id = Pm_GetDefaultInputDeviceID();
	}
	const PmDeviceInfo *info = Pm_GetDeviceInfo(id);
	if (info == NULL) {
		fprintf(stderr, "Could not open MIDI input device %d.\n", id);
		return -1;
	}

	PmError err = Pm_OpenInput(&_instream, id, NULL, INBUF_SIZE, NULL, NULL);
	if (err != pmNoError) {
		fprintf(stderr, "Could not open MIDI input stream: %s.\n",
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
    // We set the MIDI latency to one buffer's length.  TODO:  It probably needs to include
    // the buffer count value.
    long latency = long(1000.0 * RTcmix::bufsamps() / RTcmix::sr());
    midiport->start(latency);
    midiport->sendMIDIStart(0);
}

static void stopCallback(void *context)
{
#if DEBUG
    printf("RTcmixMIDIOutput stop callback called\n");
#endif
    RTcmixMIDIOutput *midiout = (RTcmixMIDIOutput*) context;
    midiout->sendMIDIStop(0);
    midiout->stop();
}

RTcmixMIDIOutput::RTcmixMIDIOutput() : _deviceID(0), _outstream(NULL)
{
    
}

RTcmixMIDIOutput::~RTcmixMIDIOutput()
{
#if DEBUG
    printf("Destroying RTcmixMIDIOutput instance\n");
#endif
    RTcmix::unregisterAudioStartCallback(startCallback, this);
    RTcmix::unregisterAudioStopCallback(stopCallback, this);
    stop();
    Pm_Terminate();
}

int RTcmixMIDIOutput::init()
{
    Pm_Initialize();
    
    const char *devname = RTOption::midiOutDevice();
#if DEBUG
    printf("Requested MIDI output device: \"%s\"\n", devname);
#endif
    if (strlen(devname)) {
        bool found = false;
        const int numdev = Pm_CountDevices();
        for (int id=0; id < numdev; id++) {
            const PmDeviceInfo *info = Pm_GetDeviceInfo(id);
            if (info->output) {
#if DEBUG
                printf("Found MIDI output device: \"%s\"\n", info->name);
#endif
                if (strcmp(info->name, devname) == 0) {
                    _deviceID = id;
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            fprintf(stderr, "WARNING: no match for MIDI output device \"%s\""
                    " ... using default.\n", devname);
            _deviceID = Pm_GetDefaultOutputDeviceID();
        }
    }
    else {
        fprintf(stderr, "WARNING: using default MIDI output device.\n");
        _deviceID = Pm_GetDefaultOutputDeviceID();
    }
    const PmDeviceInfo *info = Pm_GetDeviceInfo(_deviceID);
    if (info == NULL) {
        fprintf(stderr, "Could not open MIDI output device %d.\n", _deviceID);
        return -1;
    }
    return 0;
}

int RTcmixMIDIOutput::start(long latency)
{
#if DEBUG
    printf("Opening Portmidi output stream with latency of %ld ms\n", latency);
#endif
    PmError err = Pm_OpenOutput(&_outstream, _deviceID, NULL, INBUF_SIZE, NULL, NULL, latency);
    if (err != pmNoError) {
        fprintf(stderr, "Could not open MIDI output stream: %s.\n", Pm_GetErrorText(err));
        return -1;
    }
    return 0;
}

int RTcmixMIDIOutput::stop()
{
#if DEBUG
    printf("Closing Portmidi output stream\n");
#endif
    if (_outstream) {
        Pm_Close(_outstream);
        _outstream = NULL;
    }
    return 0;
}

inline uchar make_status(uchar type, uchar chan) { return chan | type; }

void RTcmixMIDIOutput::sendNoteOn(PmTimestamp timestamp, uchar chan, uchar pitch, uchar vel)
{
    PmEvent buffer;
    buffer.message = Pm_Message(make_status(kNoteOn, chan), pitch, vel);
    buffer.timestamp = timestamp;
    lock();
    Pm_Write(outstream(), &buffer, 1);
    unlock();
}

void RTcmixMIDIOutput::sendNoteOff(PmTimestamp timestamp, uchar chan, uchar pitch, uchar vel)
{
    PmEvent buffer;
    buffer.message = Pm_Message(make_status(kNoteOff, chan), pitch, vel);
    buffer.timestamp = timestamp;
    lock();
    Pm_Write(outstream(), &buffer, 1);
    unlock();
}

void RTcmixMIDIOutput::sendControl(PmTimestamp timestamp, uchar chan, uchar control, unsigned value)
{
    PmEvent buffer;
    buffer.message = Pm_Message(make_status(kControl, chan), control, value);
    buffer.timestamp = timestamp;
    lock();
    Pm_Write(outstream(), &buffer, 1);
    unlock();
}

void RTcmixMIDIOutput::sendPitchBend(long timestamp, uchar chan, unsigned value)
{
    PmEvent buffer;
    uchar msb = (value >> 8) & 0xff;
    uchar lsb = value & 0xff;
    buffer.message = Pm_Message(make_status(kPitchBend, chan), lsb, msb);
    buffer.timestamp = timestamp;
    lock();
    Pm_Write(outstream(), &buffer, 1);
    unlock();
}

void RTcmixMIDIOutput::sendProgramChange(PmTimestamp timestamp, uchar chan, uchar program)
{
    PmEvent buffer;
    buffer.message = Pm_Message(make_status(kProgram, chan), program, 0);
    buffer.timestamp = timestamp;
    lock();
    Pm_Write(outstream(), &buffer, 1);
    unlock();
}

void RTcmixMIDIOutput::sendMIDIStart(long timestamp)
{
    PmEvent buffer;
    buffer.message = Pm_Message(0xFA, 0, 0);
    buffer.timestamp = timestamp;
    PmEvent buffers[16];
    for (int chan = 0; chan < 16; ++chan) {
        buffers[chan].message = Pm_Message(make_status(kControl, chan), 121, 0);   // ResetAllControllers
        buffers[chan].timestamp = timestamp;
    }
    lock();
    Pm_Write(outstream(), buffers, 16);
    Pm_Write(outstream(), &buffer, 1);
    unlock();
}

void RTcmixMIDIOutput::sendMIDIStop(long timestamp)
{
    PmEvent buffer1, buffer2;
    buffer1.message = Pm_Message(0xFF, 0, 0);   // system reset
    buffer1.timestamp = timestamp;
    buffer2.message = Pm_Message(0xFC, 0, 0);   // timecode stop
    buffer2.timestamp = timestamp;
    PmEvent buffers[16];
    for (int chan = 0; chan < 16; ++chan) {
        buffers[chan].message = Pm_Message(make_status(kControl, chan), 120, 0);   // AllSoundOff
        buffers[chan].timestamp = timestamp;
    }
    lock();
    Pm_Write(outstream(), buffers, 16);
    Pm_Write(outstream(), &buffer1, 1);
    Pm_Write(outstream(), &buffer2, 1);
    unlock();
}

RTcmixMIDIOutput *createMIDIOutputPort()
{
    // We need rtsetparams in order to set the latency of the MIDI system.
    if (!RTcmix::rtsetparams_was_called()) {
        fprintf(stderr, "You must call rtsetparams before setting up MIDI output\n");
        return NULL;
    }
    RTcmixMIDIOutput *midiport = NULL;
    try {
        midiport = new RTcmixMIDIOutput();
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
