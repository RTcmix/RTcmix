/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  PROGRAM.h
//
//  Created by Douglas Scott on 1/19/20.
//

#ifndef PROGRAM_h
#define PROGRAM_h

#include "MIDIBase.h"

class PROGRAM : public MIDIBase {
    
public:
    PROGRAM();
    virtual ~PROGRAM();
    virtual int init(double *, int);
protected:
    virtual void doStart();
    virtual void doupdate(FRAMETYPE currentFrame);
    virtual void doStop(FRAMETYPE currentFrame) {}
private:
    int _patchNumber;
};


#endif /* PROGRAM_h */
