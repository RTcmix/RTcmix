/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  SYSEX.h
//
//  Created by Douglas Scott on 4/30/24.
//

#ifndef SYSEX_h
#define SYSEX_h

#include "MIDIBase.h"

class SYSEX : public MIDIBase {
    
public:
    SYSEX();
    virtual ~SYSEX();
    virtual int init(double *, int);
protected:
    virtual void doStart(FRAMETYPE frameOffset);
    virtual void doupdate(FRAMETYPE currentFrame) {}
    virtual void doStop(FRAMETYPE currentFrame) {}
private:
	unsigned char *_mesg;
};

#endif /* SYSEX_h */
