/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

// RTMIDIOutput - an internal interface for writing MIDI data in real time to a
// MIDI output port.
// Douglas Scott, Jan 2020

#ifndef RTMIDIOUTPUT_H
#define RTMIDIOUTPUT_H

class RTMIDIOutput {
public:
    virtual ~RTMIDIOutput() {}
    virtual void sendNoteOn(long timestamp, unsigned char chan, unsigned char pitch, unsigned char vel)=0;
    virtual void sendNoteOff(long timestamp, unsigned char chan, unsigned char pitch, unsigned char vel)=0;
    virtual void sendControl(long timestamp, unsigned char chan, unsigned char control, unsigned value)=0;
    virtual void sendPitchBend(long timestamp, unsigned char chan, unsigned value)=0;
    virtual void sendProgramChange(long timestamp, unsigned char chan, unsigned char program)=0;
    virtual void sendSysEx(long timestamp, unsigned char *msg)=0;
};

#endif	// RTMIDIOUTPUT_H

