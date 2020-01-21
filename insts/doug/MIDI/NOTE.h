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
    int _midiVel;
};

