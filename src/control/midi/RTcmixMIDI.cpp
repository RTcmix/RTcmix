/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <RTcmixMIDI.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <Option.h>

// if INBUF_SIZE is 0, PortMidi uses a default value
#define INBUF_SIZE		0
#define MSG_QUEUE_SIZE	32

enum {
	kQuitMsg = 0
};


RTcmixMIDI::RTcmixMIDI()
	: _instream(NULL), _outstream(NULL), _active(false)
{
	clear();
}

RTcmixMIDI::~RTcmixMIDI()
{
	long msg = kQuitMsg;
	Pm_Enqueue(_mainToMIDI, &msg);

	// wait for acknowlegement
	int spin;
	do {
		spin = Pm_Dequeue(_MIDIToMain, &msg);
	} while (spin == 0);

	Pt_Stop();	// stop timer
	if (_instream)
		Pm_Close(_instream);
	if (_outstream)
		Pm_Close(_outstream);

	Pm_QueueDestroy(_mainToMIDI);
	Pm_QueueDestroy(_MIDIToMain);

	Pm_Terminate();
}

int RTcmixMIDI::init()
{
	_MIDIToMain = Pm_QueueCreate(MSG_QUEUE_SIZE, sizeof(long));
	_mainToMIDI = Pm_QueueCreate(MSG_QUEUE_SIZE, sizeof(long));
	if (_mainToMIDI == NULL || _MIDIToMain == NULL) {
		fprintf(stderr, "Could not create MIDI message queues.\n");
		return -1;
	}

	// Start the timer before starting MIDI I/O.  The timer runs the callback
	// function every SLEEP_MSEC milliseconds.
	Pt_Start(SLEEP_MSEC, &_processMIDI, this);

	Pm_Initialize();

	int id = 0;
	const char *devname = Option::midiInDevice();
	if (devname) {
		const int numdev = Pm_CountDevices();
		for ( ; id < numdev; id++) {
			const PmDeviceInfo *info = Pm_GetDeviceInfo(id);
			if (info->input) {
				if (strcmp(info->name, devname) == 0)
					break;
			}
		}
		if (id == 0 || id == numdev) {
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

void RTcmixMIDI::clear()
{
	for (int chan = 0; chan < 16; chan++) {
		_bend[chan] = -INT_MAX;
		_program[chan] = -INT_MAX;
		_chanpress[chan] = -INT_MAX;
		for (int i = 0; i < 128; i++) {
			_noteonvel[chan][i] = -INT_MAX;
			_noteoffvel[chan][i] = -INT_MAX;
			_control[chan][i] = -INT_MAX;
			_polypress[chan][i] = -INT_MAX;
		}
	}
}


// -------------------------------------------------------------------- dump ---
// Just for debugging.

const char *RTcmixMIDI::getValueString(const int val)
{
	if (val == -INT_MAX)
		return "--";

	static char buf[16];
	snprintf(buf, 16, "%d", val);
	return buf;
}

void RTcmixMIDI::dump()
{
	printf("\nDumping current MIDI state...\n");
	for (int chan = 0; chan < 16; chan++) {
		printf("---------------------------------------- Channel %d\n", chan + 1);
		printf("   Bend:\t%s\n", getValueString(_bend[chan]));
		printf("   Program:\t%s\n", getValueString(_program[chan]));
		printf("   ChanPress:\t%s\n", getValueString(_chanpress[chan]));
		printf("   NoteOnVel:\n");
		for (int i = 0; i < 128; i++)
			printf("      [%d]:\t%s\n", i, getValueString(_noteonvel[chan][i]));
		printf("   NoteOffVel:\n");
		for (int i = 0; i < 128; i++)
			printf("      [%d]:\t%s\n", i, getValueString(_noteoffvel[chan][i]));
		printf("   Control:\n");
		for (int i = 0; i < 128; i++)
			printf("      [%d]:\t%s\n", i, getValueString(_control[chan][i]));
		printf("   PolyPress:\n");
		for (int i = 0; i < 128; i++)
			printf("      [%d]:\t%s\n", i, getValueString(_polypress[chan][i]));
		printf("\n");
	}
}


// ------------------------------------------------------------ _processMIDI ---

enum {
	kNoteOff = 0x80,
	kNoteOn = 0x90,
	kPolyPress = 0xA0,
	kControl = 0xB0,
	kProgram = 0xC0,
	kChanPress = 0xD0,
	kPitchBend = 0xE0
} StatusByte;

// This is called from the MIDI worker thread.  The RTcmixMIDI object, held by
// main thread, communicates with this worker thread by passing messages
// back and forth.
void RTcmixMIDI::_processMIDI(PtTimestamp timestamp, void *context)
{
	RTcmixMIDI *obj = (RTcmixMIDI *) context;

	// Check for messages from object.
	PmError result;
	do {
		long msg;
		result = Pm_Dequeue(obj->mainToMIDI(), &msg); 
		if (result) {
			if (msg == kQuitMsg) {
				// acknowledge receipt of quit message
				Pm_Enqueue(obj->MIDIToMain(), &msg);
				obj->active(false);
				return;
			}
		}
	} while (result);         

	// Don't poll MIDI until initialization completes.  We still listen for
	// messages, in case RTcmixMIDI::init fails and dtor then called.
	if (!obj->active())
		return;

	// See if there is any midi input to process.
	do {
		result = Pm_Poll(obj->instream());
		if (result) {
			long status, chan, data1, data2;
			PmEvent buffer;

			if (Pm_Read(obj->instream(), &buffer, 1) == pmBufferOverflow) 
				continue;

			// unless there was overflow, we should have a message now
			status = Pm_MessageStatus(buffer.message);
			data1 = Pm_MessageData1(buffer.message);
			data2 = Pm_MessageData2(buffer.message);

			chan = status & 0x0F;

			switch (status & 0xF0) {
				case kNoteOn:
					obj->setNoteOnVel(chan, data1, data2);
					break;
				case kNoteOff:
					obj->setNoteOffVel(chan, data1, data2);
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
				default:
					printf("0x%.2x, %ld, %ld\n", (u_char) status, data1, data2);
					break;
			}
		}
	} while (result);

//sleep(2); obj->dump();
}


RTcmixMIDI *createMIDIPort()
{
	RTcmixMIDI *midiport = new RTcmixMIDI();
	if (midiport) {
		if (midiport->init() == -1) {
			delete midiport;
			return NULL;
		}
	}

	return midiport;
}

