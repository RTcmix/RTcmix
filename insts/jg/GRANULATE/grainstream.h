// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include "../../../src/rtcmix/Random.h"

#define MAX_NUM_VOICES 50

class Ooscil;
class GrainVoice;

class GrainStream {

public:
   GrainStream(
      const float srate,            // sampling rate
      double *inputTable,           // table of audio frames
      int tableLen,                 // number of samples (not frames) in table
      const int numInChans,         // number of channels in table
      const int numOutChans,        // number of output chans (1 or 2)
      const bool preserveGrainDur,  // maintain grain duration, regardless of
                                    // transposition
      const int seed,               // random seed
      const bool use3rdOrderInterp  // transposition interpolation order:
                                    // 3rd-order if true; 2nd-order if false
   );
   ~GrainStream();

   inline void setInputChan(const int chan) { _inchan = chan; }

   void setGrainEnvelopeTable(double *table, int length);

   void setInskip(const double inskip);
   void setWindow(const double start, const double end);

   inline void setWraparound(const bool wrap) { _wrap = wrap; }

   void setTraversalRateAndGrainHop(const double rate, const double hop);

   inline void setInputJitter(const double jitter) {
      _maxinjitter = jitter;
      _inrand->setmin(-jitter);
      _inrand->setmax(jitter);
   }

   inline void setOutputJitter(const double jitter) {
      _maxoutjitter = jitter;
      _outrand->setmin(-jitter);
      _outrand->setmax(jitter);
   }

   inline void setGrainDuration(const double min, const double max) {
      _durrand->setmin(min);
      _durrand->setmax(max);
   }

   inline void setGrainAmp(const double min, const double max) {
      _amprand->setmin(min);
      _amprand->setmax(max);
   }

   inline void setGrainTransposition(const double transp) { _transp = transp; }

   inline void setGrainTranspositionJitter(const double jitter) {
      _maxtranspjitter = jitter;
      if (_transptab)
         _transprand->setmin(0.0);
      else
         _transprand->setmin(-jitter);
      _transprand->setmax(jitter);
   }

   void setGrainTranspositionCollection(double *table, int tableLen);

   inline void setGrainPan(const double min, const double max) {
      _panrand->setmin(min);
      _panrand->setmax(max);
   }

   // Two APIs...

   // Single-frame I/O.  Call prepare, then retrieve last computed
   // values with lastL and lastR.
   bool prepare();

   inline float lastL() const { return _lastL; }
   inline float lastR() const { return _lastR; }

   // Block I/O.  Pass a buffer sized to <numFrames> * number of output chans.
   bool processBlock(float *buffer, const int numFrames, const float amp);

private:
   const int firstFreeVoice();
   const double getTransposition();
   void playGrains();
   void playGrains(float buffer[], const int numFrames, const float amp);
   bool maybeStartGrain(const int bufoutstart = 0);

   // set in response to user input
   double _srate;
   double *_inputtab;
   int _inputframes;
   int _outchans;
   int _inchan;
   int _inskip;
   int _winstart;
   int _winend;
   bool _wrap;
   int _inhop;
   int _outhop;
   double _maxinjitter;
   double _maxoutjitter;
   double _transp;
   double _maxtranspjitter;
   double *_transptab;
   int _transplen;

   // set internally
   GrainVoice *_voices[MAX_NUM_VOICES];
   Random *_inrand;
   Random *_outrand;
   Random *_durrand;
   Random *_amprand;
   Random *_transprand;
   Random *_panrand;
   int _outframecount;
   int _nextinstart;
   int _nextoutstart;
   int _maxvoice;
   double _travrate;
   double _lasttravrate;
   double _lastinskip;
   float _lastL;
   float _lastR;
};


