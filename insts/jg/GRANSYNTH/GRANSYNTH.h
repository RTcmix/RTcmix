#include <Instrument.h>

class SynthGrainStream;

class GRANSYNTH : public Instrument {

public:
   GRANSYNTH();
   virtual ~GRANSYNTH();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();

private:
   int _nargs, _branch;
   bool _stereoOut;
   double _amp;
   SynthGrainStream *_stream;
   float *_block;

   void doupdate();
};

// update flags (shift amount is pfield index)
enum {
	kAmp = 1 << 2,
	kHopTime = 1 << 5,
	kOutJitter = 1 << 6,
	kMinDur = 1 << 7,
	kMaxDur = 1 << 8,
	kMinAmp = 1 << 9,
	kMaxAmp = 1 << 10,
	kPitch = 1 << 11,
	kPitchJitter = 1 << 13,
	kMinPan = 1 << 15,
	kMaxPan = 1 << 16
};


