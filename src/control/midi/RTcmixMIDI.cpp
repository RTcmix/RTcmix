/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <RTcmixMIDI.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

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

	_MIDIToMain = Pm_QueueCreate(MSG_QUEUE_SIZE, sizeof(long));
	_mainToMIDI = Pm_QueueCreate(MSG_QUEUE_SIZE, sizeof(long));
//FIXME: how to handle error?
	if (_mainToMIDI == NULL || _MIDIToMain == NULL) {
		fprintf(stderr, "Could not create MIDI message queues.\n");
		return;
	}

	// Start the timer before starting MIDI I/O.  The timer runs the callback
	// function every SLEEP_MSEC milliseconds.
	Pt_Start(SLEEP_MSEC, &_processMIDI, 0);

//FIXME: read device name from Option class?
	int id = Pm_GetDefaultInputDeviceID();
	const PmDeviceInfo *info = Pm_GetDeviceInfo(id);
//FIXME: how to handle error?
	if (info == NULL) {
		fprintf(stderr, "Could not open input device %d.\n", id);
		return;
	}

	PmError err = Pm_OpenInput(&_instream, id, NULL, INBUF_SIZE, NULL, NULL);
//FIXME: how to handle error?
	if (err != pmNoError) {
		fprintf(stderr, "Could not open input stream: %s.\n",
									Pm_GetErrorText(err));
		return;
	}

	Pm_SetFilter(_instream, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX);

	// Empty buffer after setting filter, in case anything got through before.
	PmEvent buffer;
	while (Pm_Poll(_instream))
		Pm_Read(_instream, &buffer, 1);

	// enable processing in MIDI thread (_processMIDI)
	active(true);

	Pm_Initialize();
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
}

void RTcmixMIDI::clear()
{
	for (int chan = 0; chan < 16; chan++) {
		_bend[chan] = -1;
		_program[chan] = -1;
		_chanpress[chan] = -1;
		for (int i = 0; i < 128; i++) {
			_noteonvel[chan][i] = -1;
			_noteoffvel[chan][i] = -1;
			_control[chan][i] = -1;
			_polypress[chan][i] = -1;
		}
	}
}

void RTcmixMIDI::dump()
{
	printf("\nDumping current MIDI state...\n");
	for (int chan = 0; chan < 16; chan++) {
		printf("---------------------------------------- Channel %d\n", chan + 1);
		printf("   Bend:\t%d\n", _bend[chan]);
		printf("   Program:\t%d\n", _program[chan]);
		printf("   ChanPress:\t%d\n", _chanpress[chan]);
		printf("   NoteOnVel:\n");
		for (int i = 0; i < 128; i++)
			printf("      [%d]:\t%d\n", i, _noteonvel[chan][i]);
		printf("   NoteOffVel:\n");
		for (int i = 0; i < 128; i++)
			printf("      [%d]:\t%d\n", i, _noteoffvel[chan][i]);
		printf("   Control:\n");
		for (int i = 0; i < 128; i++)
			printf("      [%d]:\t%d\n", i, _control[chan][i]);
		printf("   PolyPress:\n");
		for (int i = 0; i < 128; i++)
			printf("      [%d]:\t%d\n", i, _polypress[chan][i]);
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

	// do nothing until initialization completes
	if (!obj->active())
		return;

	// check for messages from object
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

	// see if there is any midi input to process
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
}


// This is kinda dumb, but we do it this way to be consistent with those
// connection types that have platform-dependent derived classes, such as mouse.

RTcmixMIDI *createMIDIPort()
{
	RTcmixMIDI *midiport = new RTcmixMIDI();

	return midiport;
}

