// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include <math.h>

class Ooscil;

class GrainVoice {

public:
   GrainVoice(const double srate, double *inputTable, int inputFrames,
      const int numInChans, const int numOutChans, const bool preserveDur,
      const bool use3rdOrderInterp);
   ~GrainVoice();

   inline bool inUse() const { return _inuse; }

   void setGrainEnvelopeTable(double *table, int length);

   void startGrain(const int bufoutstart, const int instartframe,
      const double outdur, const int inchan, const double amp,
      const double transp, const double pan, const bool forwards);

   // single frame version
   void next(float &left, float &right);

   // block version
   void next(float *buffer, const int numFrames, const float amp);

private:
   float getSigNoTransp();
   float getSig2ndOrder();
   float getSig3rdOrder();

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
   double *_inputtab;
   int _inputframes;
   int _numinchans, _numoutchans;
   bool _preservedur;
   bool _interp3;

   Ooscil *_env;
   bool _inuse, _getflag, _forwards;
   int _inchan, _bufoutstart, _instartframe, _inendframe, _incurframe;
   float _oldersig, _oldsig, _newsig, _newestsig, _amp, _pan;
   double _increment, _counter, _cpsoct10;
};

