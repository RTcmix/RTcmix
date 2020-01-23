/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */

#ifndef _NOTE_h
#define _NOTE_h

#include "MIDIBase.h"

class NOTE : public MIDIBase {

public:
	NOTE();
	virtual ~NOTE();
	virtual int init(double *, int);
protected:
    virtual void doStart();
    virtual void doupdate(FRAMETYPE currentFrame);
    virtual void doStop(FRAMETYPE currentFrame);
private:
    int _midiNote;
    float _midiVel;
};

#endif // _NOTE_h


