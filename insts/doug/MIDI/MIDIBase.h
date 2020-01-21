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
    double _amp, _pan;
    FRAMETYPE _runStartFrame;   // first frame of current run()
    RTMIDIOutput *_outputPort;
};

