// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include <math.h>

class Ooscili;

class SynthGrainVoice {

public:
   SynthGrainVoice(const double srate, double *waveTable, int tableLen,
      const int numOutChans);
   ~SynthGrainVoice();

   inline bool inUse() const { return _inuse; }

   void setGrainEnvelopeTable(double *table, int length);

   void startGrain(const int bufoutstart, const double startphase,
      const double outdur, const double amp, const double pitch,
      const double pan);

   // single frame version
   void next(float &left, float &right);

   // block version
   void next(float *buffer, const int numFrames, const float amp);

private:

   // Compensate for hole in the middle.
   inline float boost(const float panL, const float panR)
   {
      return 1.0 / sqrt((panL * panL) + (panR * panR));
   }

   // Called only when doing block I/O.
   inline void addOutGrain(float sig, float *buffer, const int frameIndex)
   {
      sig *= _env->next() * _amp;

      if (_numoutchans > 1) {
         const float panR = 1.0 - _pan;
         sig *= boost(_pan, panR);
         const int index = frameIndex * _numoutchans;
         buffer[index] += sig * _pan;
         buffer[index + 1] += sig * panR;
      }
      else
         buffer[frameIndex] += sig;
   }

   double _srate;
   double *_wavetab;
   int _wavetablen;
   int _numoutchans;

   Ooscili *_osc;
   Ooscili *_env;
   bool _inuse;
   int _bufoutstart, _outframes, _curframe;
   float _amp, _pan;
};

