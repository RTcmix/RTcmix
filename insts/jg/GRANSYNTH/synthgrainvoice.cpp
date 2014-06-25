// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include <stdio.h>
#include <Ougens.h>
#include <ugens.h>   // for octpch, cpsoct
#include "synthgrainvoice.h"
//#define NDEBUG     // disable asserts
#include <assert.h>

#define DEBUG 0


// NOTE: We don't own the table memory.
SynthGrainVoice::SynthGrainVoice(const double srate, double *waveTable,
	int tableLen, const int numOutChans)
   : _srate(srate), _wavetab(waveTable), _wavetablen(tableLen),
     _numoutchans(numOutChans), _osc(NULL), _env(NULL),
     _inuse(false), _amp(0.0), _pan(0.0)
{
   assert(_srate > 0.0);
   assert(_wavetab != NULL);
   assert(_wavetablen > 1);

   _osc = new Ooscili(_srate, 1.0, waveTable, tableLen);
}

SynthGrainVoice::~SynthGrainVoice()
{
   delete _osc;
   delete _env;
}

// NOTE: We don't own the table memory, just the envelope object.
void SynthGrainVoice::setGrainEnvelopeTable(double *table, int length)
{
   delete _env;
   _env = new Ooscili(_srate, 1.0, table, length);
}

// <pitch> is in linear octaves.
void SynthGrainVoice::startGrain(const int bufoutstart, const double startphase,
   const double outdur, const double amp, const double pitch, const double pan)
{
   _bufoutstart = bufoutstart;
   _outframes = int((outdur * _srate) + 0.5);
   _curframe = 0;

   assert(_osc != NULL);
   _osc->setfreq(cpsoct(pitch));
   _osc->setphase(startphase);

   assert(_env != NULL);
   _env->setfreq(1.0 / outdur);
   _env->setphase(0.0);

   _amp = amp;
   _pan = pan;

   _inuse = true;

#if DEBUG > 0
   printf("outdur=%f (frames=%d) pitch=%f, amp=%f, pan=%f\n",
      outdur, _outframes, pitch, _amp, _pan);
#endif
}

// If output is mono, <left> holds signal, and <right> is zero.
void SynthGrainVoice::next(float &left, float &right)
{
   float sig = _osc->next();
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

   _curframe++;
   if (_curframe == _outframes)
      _inuse = false;
}

void SynthGrainVoice::next(float *buffer, const int numFrames, const float amp)
{
	for (int i = _bufoutstart; i < numFrames; i++) {
		float sig = _osc->next() * amp;
		addOutGrain(sig, buffer, i);           // inlined

      _curframe++;
      if (_curframe == _outframes) {
         _inuse = false;
         break;
      }
	}
   _bufoutstart = 0;
}

