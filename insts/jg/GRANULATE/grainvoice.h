// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

class Ooscil;

class GrainVoice {

public:
   GrainVoice(const double srate, double *inputTable, int inputFrames,
      const int numInChans, const int numOutChans, const bool preserveDur);
   ~GrainVoice();

   inline bool inUse() const { return _inuse; }

   void setGrainEnvelopeTable(double *table, int length);

   void startGrain(const int instartframe, const double outdur,
      const int inchan, const double amp, const double transp,
      const double pan);

   void next(float &left, float &right, const bool forwards);

private:
   float getSigNoTransp(const bool forwards);
   float getSig2ndOrder(const bool forwards);

   double _srate;
   double *_inputtab;
   int _inputframes;
   int _numinchans, _numoutchans;
   bool _preservedur;

   Ooscil *_env;
   bool _inuse, _getflag;
   int _inchan, _instartframe, _inendframe, _incurframe;
   float _oldersig, _oldsig, _newsig, _amp, _pan;
   double _increment, _counter, _cpsoct10;
};

