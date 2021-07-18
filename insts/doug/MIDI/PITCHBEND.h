/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  PITCHBEND.h
//
//  Created by Douglas Scott on 1/24/20.
//

#ifndef PITCHBEND_h
#define PITCHBEND_h

#include "MIDIBase.h"

class PITCHBEND : public MIDIBase {
    
public:
    PITCHBEND();
    virtual ~PITCHBEND();
    virtual int init(double *, int);
protected:
    virtual void doStart(FRAMETYPE frameOffset);
    virtual void doupdate(FRAMETYPE currentFrame);
    virtual void doStop(FRAMETYPE currentFrame) {}
private:
    float _bendValue;
};


#endif /* PITCHBEND_h */
