/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */

#ifndef _MIDIBase_h
#define _MIDIBase_h

#include <Instrument.h>      // the base class for this instrument

class RTMIDIOutput;

class MIDIBase : public Instrument {

public:
	MIDIBase();
	virtual ~MIDIBase();
	virtual int init(double *, int);
    virtual int configure();
	virtual int run();
protected:
    virtual void doStart()=0;
    virtual void doupdate(FRAMETYPE currentFrame)=0;
    virtual void doStop(FRAMETYPE currentFrame)=0;
    FRAMETYPE getRunStartFrame() const { return _runStartFrame; };
protected:
    int _branch;
    int _midiChannel;
    int _nargs;
    FRAMETYPE _runStartFrame;   // first frame of current run()
    RTMIDIOutput *_outputPort;
};

#endif  // _MIDIBase_h

