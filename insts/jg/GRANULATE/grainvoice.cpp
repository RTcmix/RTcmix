// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.
//
// The transposition code comes from Doug Scott's TRANS instrument.

#include <stdio.h>
#include <assert.h>
#include <Ougens.h>
#include <ugens.h>   // for octpch, cpsoct
#include "grainvoice.h"

#define DEBUG 0

//#define NDEBUG     // disable asserts


// NOTE: We don't own the table memory.
GrainVoice::GrainVoice(const double srate, double *inputTable, int inputFrames,
   const int numInChans, const int numOutChans, const bool preserveDur)
   : _srate(srate), _inputtab(inputTable), _inputframes(inputFrames),
     _numinchans(numInChans), _numoutchans(numOutChans),
     _preservedur(preserveDur), _env(NULL), _inuse(false), _amp(0.0), _pan(0.0)
{
   assert(_srate > 0.0);
   assert(_inputtab != NULL);
   assert(_inputframes > 1);
   assert(_numinchans > 0);

   _cpsoct10 = cpsoct(10.0);
}

GrainVoice::~GrainVoice()
{
   delete _env;
}

// NOTE: We don't own the table memory, just the envelope object.
void GrainVoice::setGrainEnvelopeTable(double *table, int length)
{
   delete _env;
   _env = new Ooscil(_srate, 1.0, table, length);
}

// <transp> is in linear octaves, relative to zero.
void GrainVoice::startGrain(const int instartframe, const double outdur,
   const int inchan, const double amp, const double transp, const double pan)
{
   assert(inchan < _numinchans);

   // Getting durations right is tricky when transposing, because our
   // transposition method doesn't preserve duration.  For example, if we
   // request 1 second and transpose down an octave, then the transposer will
   // generate 2 seconds, while consuming 1 second of the source audio.  We
   // let the client decide whether to preserve <outdur> when transposing.
   // If _preservedur is true, then we adjust the duration fed to the 
   // transposer so that the result will be <outdur>.  In the previous example,
   // we would tell the transposer to consume only a half second of audio
   // so that the result would last 1 second.  The grain envelope spans
   // <outdur>, regardless of _preservedur state.
   // 
   // There's another problem.  If a grain duration would cause us to read past
   // the end of the input array, then we have to adjust its duration, while
   // taking into account the transposition issues.

   // Set up transposition increment (for interpolation pointer).
   if (transp == 0.0)
      _increment = 1.0;
   else
      _increment = cpsoct(10.0 + transp) / _cpsoct10;

   const double inputdur = _preservedur ? outdur * _increment : outdur;
   const double outputdur = _preservedur ? outdur : outdur / _increment;

   int inframes = int((inputdur * _srate) + 0.5);

   _instartframe = instartframe;
   _inendframe = _instartframe + inframes;

#ifdef NOTNOW
   if (_inendframe >= _inputframes) {
      _inendframe = _inputframes - 1;
      inframes = _inendframe - _instartframe;
      outputdur = double(inframes) / _srate;
      if (_preservedur)
         outputdur *= _increment;
   }
   if (inframes <= 0)
      return;
#else
   // If inputdur would cause us to read past the end of the input array,
   // don't play this grain.
   if (_inendframe >= _inputframes) {
 #if DEBUG > 0
      printf("Suppressing grain that would run past input array.\n");
 #endif
      return;
#endif
   }

   assert(_env != NULL);
   _env->setfreq(1.0 / outputdur);
   _env->setphase(0.0);

   _inchan = inchan;
   _amp = amp;
   _pan = pan;

   _oldsig = 0.0f;
   _newsig = 0.0f;

   _getflag = true;
   _incurframe = _instartframe;
   _counter = double(_instartframe);

   _inuse = true;

#if DEBUG > 0
   printf("inputdur=%f, outputdur=%f, inputframes=%d, startframe=%d, \
      endframe=%d, inchan=%d, numinchans=%d\n",
      inputdur, outputdur, _inputframes, _instartframe, _inendframe,
      _inchan, _numinchans);
#endif
}

float GrainVoice::getSigNoTransp(const bool forwards)
{
   assert(_incurframe >= 0);
   assert(_incurframe < _inputframes);

   const int index = (_incurframe * _numinchans) + _inchan;

   if (forwards) {
      _incurframe++;
      if (_incurframe >= _inendframe)
         _inuse = false;            // returning last frame for this grain
   }
   else {
      _incurframe--;
      if (_incurframe < _instartframe)
         _inuse = false;
   }
   return float(_inputtab[index]);
}

inline float _interp2ndOrder(float y0, float y1, float y2, float t)
{
   float hy0 = y0 * 0.5f;
   float hy2 = y2 * 0.5f;
   float b = (-3.0f * hy0) + (2.0f * y1) - hy2;
   float c = hy0 - y1 + hy2;

   return y0 + (b * t) + (c * t * t);
}

float GrainVoice::getSig2ndOrder(const bool forwards)
{
   int index = (_incurframe * _numinchans) + _inchan;

   while (_getflag) {
      _oldersig = _oldsig;
      _oldsig = _newsig;

      assert(_incurframe >= 0);
      assert(_incurframe < _inputframes);

      _newsig = (float) _inputtab[index];
      index += _numinchans;

      if (forwards) {
         _incurframe++;
         if (_incurframe >= _inendframe) {
            _inuse = false;         // returning last frame for this grain
            break;
         }
      }
      else {
         _incurframe--;
         if (_incurframe < _instartframe) {
            _inuse = false;
            break;
         }
      }
      if (_counter - double(_incurframe) < 0.5)
         _getflag = false;
   }
   const double frac = _counter - (double) _incurframe + 2.0;
   float sig = _interp2ndOrder(_oldersig, _oldsig, _newsig, frac);
   _counter += _increment;
   if (_counter - (double) _incurframe >= -0.5)
      _getflag = true;
   return sig;
}

#ifdef NOTYET
// see TRANS3.cpp
float GrainVoice::getSig3rdOrder(const bool forwards)
{
}
#endif

// Compensate for hole in the middle.
inline float _boost(const float panL, const float panR)
{
   return 1.0 / sqrt((panL * panL) + (panR * panR));
}

// If output is mono, <left> holds signal, and <right> is zero.
void GrainVoice::next(float &left, float &right, const bool forwards)
{
//FIXME: use function pointers to select interp method in startGrain
   float sig;
   if (_increment == 1.0)
      sig = getSigNoTransp(forwards);
   else
      sig = getSig2ndOrder(forwards);
   sig *= _env->next() * _amp;
   if (_numoutchans > 1) {
      const float panR = 1.0 - _pan;
      sig *= _boost(_pan, panR);
      left = sig * _pan;
      right = sig * panR;
   }
   else {
      left = sig;
      right = 0.0f;
   }
}

