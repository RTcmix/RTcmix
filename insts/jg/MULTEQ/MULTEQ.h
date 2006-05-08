#include <Ougens.h>

#define MAXBAND  8
#define MAXCHAN  8

class EQBand {
   Oequalizer  *_eq;
   OeqType     _type;
   float       _freq;
   float       _Q;
   float       _gain;
   bool        _bypass;
public:
   EQBand(float srate, OeqType type, float freq, float Q, float gain,
                                                            bool bypass);
   ~EQBand();
   inline void setparams(OeqType type, float freq, float Q, float gain,
                                                            bool bypass) {
      if (type == OeqInvalid)
         type = _type;
      if (type != _type || freq != _freq || Q != _Q || gain != _gain) {
         _type = type;
         _freq = freq;
         _Q = Q;
         _gain = gain;
         _eq->settype(_type);
         _eq->setparams(_freq, _Q, _gain);
      }
      _bypass = bypass;
   }
   inline float next(float sig) { return _bypass ? sig : _eq->next(sig); }
};


class MULTEQ : public Instrument {
   int         nargs, skip, branch, insamps, numbands;
   bool        bypass;
   float       amp;
   float       *in;
   EQBand      *eq[MAXBAND * MAXCHAN];

   OeqType getEQType(bool trystring, int pfindex);
   void doupdate();
public:
   MULTEQ();
   virtual ~MULTEQ();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

