//
//  CONTROLLER.h
//  RTcmix Desktop
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
    virtual void doStart();
    virtual void doupdate(FRAMETYPE currentFrame);
    virtual void doStop(FRAMETYPE currentFrame) {}
private:
    int _controllerNumber;
    float _controllerValue;
};


#endif /* CONTROLLER_h */
