#include <Instrument.h>

class GrainStream;

class GRANULATE : public Instrument {

public:
   GRANULATE();
   virtual ~GRANULATE();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();

private:
   int _nargs, _branch;
   bool _stereoOut;
   double _amp, _curwinstart, _curwinend;
   GrainStream *_stream;
   float *_block;
   bool _keepgoing, _stopped;

   void doupdate();
};

// update flags (shift amount is pfield index)
enum {
	kInskip = 1 << 1,
	kAmp = 1 << 3,
	kInChan = 1 << 6,
	kWinStart = 1 << 7,
	kWinEnd = 1 << 8,
	kWrap = 1 << 9,
	kTraversal = 1 << 10,
	kHopTime = 1 << 12,
	kInJitter = 1 << 13,
	kOutJitter = 1 << 14,
	kMinDur = 1 << 15,
	kMaxDur = 1 << 16,
	kMinAmp = 1 << 17,
	kMaxAmp = 1 << 18,
	kTransp = 1 << 19,
	kTranspJitter = 1 << 21,
	kMinPan = 1 << 23,
	kMaxPan = 1 << 24
};


