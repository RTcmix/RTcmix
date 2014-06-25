// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.
//
// The transposition code comes from Doug Scott's TRANS instrument.

#include <stdio.h>
#include <Ougens.h>
#include <ugens.h>   // for octpch, cpsoct
#include "grainvoice.h"
//#define NDEBUG     // disable asserts
#include <assert.h>

#define DEBUG 0


// NOTE: We don't own the table memory.
GrainVoice::GrainVoice(const double srate, double *inputTable, int inputFrames,
   const int numInChans, const int numOutChans, const bool preserveDur,
   const bool use3rdOrderInterp)
   : _srate(srate), _inputtab(inputTable), _inputframes(inputFrames),
     _numinchans(numInChans), _numoutchans(numOutChans),
     _preservedur(preserveDur), _interp3(use3rdOrderInterp), _env(NULL),
     _inuse(false), _amp(0.0), _pan(0.0)
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
void GrainVoice::startGrain(const int bufoutstart, const int instartframe,
   const double outdur, const int inchan, const double amp, const double transp,
   const double pan, const bool forwards)
{
   assert(inchan < _numinchans);

   _forwards = forwards;
   _instartframe = instartframe;
   _bufoutstart = bufoutstart;

   // Getting durations right is tricky when transposing, because our
   // transposition method doesn't preserve duration.  For example, if we
   // request 1 second and transpose down an octave, then the transposer will
   // generate 2 seconds, while consuming 1 second of the source audio.  We
   // let the caller decide whether to preserve <outdur> when transposing.
   // If _preservedur is true, then we adjust the duration fed to the 
   // transposer so that the result will be <outdur>.  In the previous example,
   // we would tell the transposer to consume only a half second of audio
   // so that the result would last 1 second.  The grain envelope spans
   // <outdur>, regardless of _preservedur state.
   // 
   // There's another problem.  If a grain duration would cause us to read past
   // the end of the input array, then we bail out without setting the _inuse
   // flag (which tells the GrainStream object whether this grain is active).

   // Set up transposition increment (for interpolation pointer).
   if (transp == 0.0)
      _increment = 1.0;
   else {
      _increment = cpsoct(10.0 + transp) / _cpsoct10;
      _oldsig = 0.0f;
      _newsig = 0.0f;
      _newestsig = 0.0f;      // for 3rd order interp only
      _getflag = true;
      _counter = double(_instartframe);
   }

   const double inputdur = _preservedur ? outdur * _increment : outdur;
   const double outputdur = _preservedur ? outdur : outdur / _increment;

   int inframes = int((inputdur * _srate) + 0.5);

   if (forwards) {
      _inendframe = _instartframe + inframes;
      // If inputdur would cause us to read past the end of the input array,
      // don't play this grain.
      if (_inendframe >= _inputframes) {
#if DEBUG > 0
         printf("Suppressing grain that would run past input array end.\n");
#endif
         return;     // without setting _inuse flag
      }
   }
   else {
      _inendframe = _instartframe - inframes;
      if (_inendframe < 0) {
#if DEBUG > 0
         printf("Suppressing grain that would run past input array start.\n");
#endif
         return;
      }
   }

   assert(_env != NULL);
   _env->setfreq(1.0 / outputdur);
   _env->setphase(0.0);

   _inchan = inchan;
   _amp = amp;
   _pan = pan;

   _incurframe = _instartframe;

   _inuse = true;

#if DEBUG > 0
   printf("inputdur=%f, outputdur=%f, _inchan=%d, _numinchans=%d, \
      _inputframes=%d, _instartframe=%d, _inendframe=%d, inframes=%d\n",
      inputdur, outputdur, _inchan, _numinchans, _inputframes, _instartframe,
      _inendframe, inframes);
#endif
}


float GrainVoice::getSigNoTransp()
{
   assert(_incurframe >= 0);
   assert(_incurframe < _inputframes);

   const int index = (_incurframe * _numinchans) + _inchan;

   if (_forwards) {
      _incurframe++;
      if (_incurframe >= _inendframe)
         _inuse = false;            // returning last frame for this grain
   }
   else {
      _incurframe--;
      if (_incurframe <= _inendframe)
         _inuse = false;
   }
   return (float) _inputtab[index];
}


inline float _interp2ndOrder(float y0, float y1, float y2, float t)
{
   const float hy0 = y0 * 0.5f;
   const float hy2 = y2 * 0.5f;
   const float b = (-3.0f * hy0) + (2.0f * y1) - hy2;
   const float c = hy0 - y1 + hy2;

   return y0 + (b * t) + (c * t * t);
}

float GrainVoice::getSig2ndOrder()
{
   int index = (_incurframe * _numinchans) + _inchan;

   while (_getflag) {
      _oldersig = _oldsig;
      _oldsig = _newsig;

      assert(_incurframe >= 0);
      assert(_incurframe < _inputframes);

      _newsig = (float) _inputtab[index];

      if (_forwards) {
         index += _numinchans;
         _incurframe++;
         if (_incurframe >= _inendframe) {
            _inuse = false;         // returning last frame for this grain
            break;
         }
         if (_counter - double(_incurframe) < 0.5)
            _getflag = false;
      }
      else {
         index -= _numinchans;
         _incurframe--;
         if (_incurframe <= _inendframe) {
            _inuse = false;
            break;
         }
         if (double(_incurframe) - _counter < 0.5)
            _getflag = false;
      }
   }
   float sig;
   if (_forwards) {
      const double frac = (_counter - (double) _incurframe) + 2.0;
      sig = _interp2ndOrder(_oldersig, _oldsig, _newsig, frac);
      _counter += _increment;
      if (_counter - (double) _incurframe >= -0.5)
         _getflag = true;
   }
   else {
      const double frac = ((double) _incurframe - _counter) + 2.0;
      sig = _interp2ndOrder(_oldersig, _oldsig, _newsig, frac);
      _counter -= _increment;
      if ((double) _incurframe - _counter >= -0.5)
         _getflag = true;
   }
   return sig;
}


inline float _interp3rdOrder(float ym2, float ym1, float yp1, float yp2,
   float t)
{
   const float a = t + 1.0f;
   const float c = t - 1.0f;
   const float d = t - 2.0f;

   const float e = a * t;
   const float f = c * d;

   return 0.5f * (a * f * ym1 - e * d * yp1) 
            + 0.166666666667f * (e * c * yp2 - t * f * ym2);
}

float GrainVoice::getSig3rdOrder()
{
   int index = (_incurframe * _numinchans) + _inchan;

   while (_getflag) {
      _oldersig = _oldsig;
      _oldsig = _newsig;
      _newsig = _newestsig;

      assert(_incurframe >= 0);
      assert(_incurframe < _inputframes);

      _newestsig = (float) _inputtab[index];

      if (_forwards) {
         index += _numinchans;
         _incurframe++;
         if (_incurframe >= _inendframe) {
            _inuse = false;         // returning last frame for this grain
            break;
         }
         if (_counter - double(_incurframe) < 0.0)
            _getflag = false;
      }
      else {
         index -= _numinchans;
         _incurframe--;
         if (_incurframe <= _inendframe) {
            _inuse = false;
            break;
         }
         if (double(_incurframe) - _counter < 0.0)
            _getflag = false;
      }
   }
   float sig;
   if (_forwards) {
      const double frac = (_counter - (double) _incurframe) + 1.0;
      sig = _interp3rdOrder(_oldersig, _oldsig, _newsig, _newestsig, frac);
      _counter += _increment;
      if (_counter - (double) _incurframe >= 0.0)
         _getflag = true;
   }
   else {
      const double frac = ((double) _incurframe - _counter) + 1.0;
      sig = _interp3rdOrder(_oldersig, _oldsig, _newsig, _newestsig, frac);
      _counter -= _increment;
      if ((double) _incurframe - _counter >= 0.0)
         _getflag = true;
   }
   return sig;
}


// If output is mono, <left> holds signal, and <right> is zero.

void GrainVoice::next(float &left, float &right)
{
   float sig;

   // Select appropriate interpolation function.  We could make these static
   // methods, and install a function pointer during startGrain, but then we'd
   // have to pass object state in and out of them.
   if (_increment == 1.0)
      sig = getSigNoTransp();
   else if (_interp3)
      sig = getSig3rdOrder();
   else
      sig = getSig2ndOrder();
   sig *= _env->next() * _amp;

   if (_numoutchans > 1) {
      const float panR = 1.0 - _pan;
      sig *= boost(_pan, panR);
      left = sig * _pan;
      right = sig * panR;
   }
   else {
      left = sig;
      right = 0.0f;
   }
}


void GrainVoice::next(float *buffer, const int numFrames, const float amp)
{
   if (_increment == 1.0) {
      for (int i = _bufoutstart; i < numFrames; i++) {
         float sig = getSigNoTransp() * amp;
         addOutGrain(sig, buffer, i);           // inlined
         if (!_inuse)
            break;
      }
   }
   else if (_interp3) {
      for (int i = _bufoutstart; i < numFrames; i++) {
         float sig = getSig3rdOrder() * amp;
         addOutGrain(sig, buffer, i);
         if (!_inuse)
            break;
      }
   }
   else {
      for (int i = _bufoutstart; i < numFrames; i++) {
         float sig = getSig2ndOrder() * amp;
         addOutGrain(sig, buffer, i);
         if (!_inuse)
            break;
      }
   }
   _bufoutstart = 0;
}


