// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include "../../../src/rtcmix/Random.h"

#define MAX_NUM_VOICES 50

class Ooscil;
class SynthGrainVoice;

class SynthGrainStream {

public:
   SynthGrainStream(
      const float srate,            // sampling rate
      double *waveTable,            // oscillator waveform table
      int tableLen,                 // number of samples in table
      const int numOutChans,        // number of output chans (1 or 2)
      const int seed                // random seed
   );
   ~SynthGrainStream();

   void setGrainEnvelopeTable(double *table, int length);

   // <hop> is the number of seconds to skip on the output before starting a
   // new grain.  We add jitter to this amount in maybeStartGrain().
   inline void setGrainHop(const double hop) {
      _hop = int((hop * _srate) + 0.5);
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

   // <pitch> is in linear octaves
   inline void setGrainPitch(const double pitch) { _pitch = pitch; }

   inline void setGrainPitchJitter(const double jitter) {
      _maxpitchjitter = jitter;
      if (_transptab)
         _pitchrand->setmin(0.0);
      else
         _pitchrand->setmin(-jitter);
      _pitchrand->setmax(jitter);
   }

   // table values are oct.pc transpositions of grain pitch.
   void setGrainTranspositionCollection(double *table, int tableLen);

   inline void setGrainPan(const double min, const double max) {
      _panrand->setmin(min);
      _panrand->setmax(max);
   }

   // Two APIs...

   // Single-frame I/O.  Call prepare, then retrieve last computed
   // values with lastL and lastR.
   void prepare();

   inline float lastL() const { return _lastL; }
   inline float lastR() const { return _lastR; }

   // Block I/O.  Pass a buffer sized to <numFrames> * number of output chans.
   void processBlock(float *buffer, const int numFrames, const float amp);

private:
   const int firstFreeVoice();
   const double getPitch();
   void playGrains();
   void playGrains(float buffer[], const int numFrames, const float amp);
   void maybeStartGrain(const int bufoutstart = 0);

   // set in response to user input
   double _srate;
   double *_wavetab;
   int _wavetablen;
   int _outchans;
   int _hop;
   double _maxoutjitter;
   double _pitch;
   double _maxpitchjitter;
   double *_transptab;
   int _transplen;

   // set internally
   SynthGrainVoice *_voices[MAX_NUM_VOICES];
   Random *_outrand;
   Random *_durrand;
   Random *_amprand;
   Random *_pitchrand;
   Random *_panrand;
   int _outframecount;
   int _nextoutstart;
   int _maxvoice;
   float _lastL;
   float _lastR;
};


