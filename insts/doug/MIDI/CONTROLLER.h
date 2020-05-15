/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  CONTROLLER.h
//
//  Created by Douglas Scott on 1/19/20.
//

#ifndef CONTROLLER_h
#define CONTROLLER_h

#include "MIDIBase.h"

class CONTROLLER : public MIDIBase {
    
public:
    CONTROLLER();
    virtual ~CONTROLLER();
    virtual int init(double *, int);
protected:
    virtual void doStart(FRAMETYPE frameOffset);
    virtual void doupdate(FRAMETYPE currentFrame);
    virtual void doStop(FRAMETYPE currentFrame) {}
private:
    int _controllerNumber;
    float _controllerValue;
};


#endif /* CONTROLLER_h */
