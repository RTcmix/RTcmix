// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <ugens.h>   // for octpch and ampdb
#include <Ougens.h>  // for Ooscil
#include "synthgrainstream.h"
#include "synthgrainvoice.h"
//#define NDEBUG       // disable asserts
#include <assert.h>

#define DEBUG 0

#define ALL_VOICES_IN_USE  -1

#define COUNT_VOICES


// NOTE: We don't own the table memory.
SynthGrainStream::SynthGrainStream(const float srate, double *waveTable,
	int tableLen, const int numOutChans, const int seed)
   : _srate(srate), _wavetab(waveTable), _wavetablen(tableLen),
     _outchans(numOutChans), _hop(0), _maxoutjitter(0.0),
     _pitch(8.0), _maxpitchjitter(0.0), _transptab(NULL), _transplen(0),
     _outframecount(0), _nextoutstart(0), _lastL(0.0f), _lastR(0.0f)
{
   for (int i = 0; i < MAX_NUM_VOICES; i++)
      _voices[i] = new SynthGrainVoice(_srate, _wavetab, _wavetablen,
									numOutChans);
   _outrand = new LinearRandom(0.0, 1.0, seed * 2);
   _durrand = new LinearRandom(0.0, 1.0, seed * 3);
   _amprand = new LinearRandom(0.0, 1.0, seed * 4);
   _pitchrand = new LinearRandom(0.0, 1.0, seed * 5);
   _panrand = new LinearRandom(0.0, 1.0, seed * 6);
#ifdef COUNT_VOICES
   _maxvoice = -1;
#endif
}


SynthGrainStream::~SynthGrainStream()
{
   for (int i = 0; i < MAX_NUM_VOICES; i++)
      delete _voices[i];
   delete [] _transptab;
   delete _outrand;
   delete _durrand;
   delete _amprand;
   delete _pitchrand;
   delete _panrand;
#ifdef COUNT_VOICES
   rtcmix_advise("GRANULATE", "Used %d voices", _maxvoice + 1);
#endif
}


// NOTE: We don't own the table memory.
// Call this before ever calling prepare or processBlock.

void SynthGrainStream::setGrainEnvelopeTable(double *table, int length)
{
   for (int i = 0; i < MAX_NUM_VOICES; i++)
      _voices[i]->setGrainEnvelopeTable(table, length);
}


void SynthGrainStream::setGrainTranspositionCollection(double *table,
	int length)
{
   delete [] _transptab;
   if (table) {
      _transptab = new double [length];
      for (int i = 0; i < length; i++)
         _transptab[i] = octpch(table[i]);
      _transplen = length;
      _pitchrand->setmin(0.0);
   }
   else {
      _transptab = NULL;
      _transplen = 0;
   }
}


// Given _pitch and (possibly) _transptab, both in linear octaves, return
// grain frequency in Hz.

const double SynthGrainStream::getPitch()
{
   double pitch_linoct = _pitch;
   const double pitchjitter = (_maxpitchjitter == 0.0) ? 0.0
                                             : _pitchrand->value();
   if (_transptab) {
      // Constrain <pitchjitter> to nearest member of transposition collection.
      double min = DBL_MAX;
      int closest = 0;
      for (int i = 0; i < _transplen; i++) {
         const double proximity = fabs(_transptab[i] - pitchjitter);
         if (proximity < min) {
            min = proximity;
            closest = i;
         }
#if DEBUG > 2
         printf("transtab[%d]=%f, jitter=%f, proximity=%f, min=%f\n",
                           i, _transptab[i], pitchjitter, proximity, min);
#endif
      }
      pitch_linoct += _transptab[closest];
#if DEBUG > 1
      printf("transpcoll chosen: %f (linoct) at index %d\n",
                                             _transptab[closest], closest);
#endif
   }
   else {
      pitch_linoct += pitchjitter;
#if DEBUG > 1
      printf("pitch (linoct): %f\n", pitch_linoct);
#endif
   }

   return pitch_linoct;
}


// Return index of first freq grain voice, or -1 if none are free.
const int SynthGrainStream::firstFreeVoice()
{
   for (int i = 0; i < MAX_NUM_VOICES; i++) {
      if (_voices[i]->inUse() == false)
         return i;
   }
   return ALL_VOICES_IN_USE;
}


// Decide whether to (re)initialize a grain.
// <bufoutstart> is the offset into the output buffer for the grain to start.
// This is relevant only when using block I/O.

void SynthGrainStream::maybeStartGrain(const int bufoutstart)
{
   if (_outframecount >= _nextoutstart) {    // time to start another grain

      const int voice = firstFreeVoice();
      if (voice != ALL_VOICES_IN_USE) {
#ifdef COUNT_VOICES
         if (voice > _maxvoice)
            _maxvoice = voice;
#endif
         const double startphase = 0.0; // XXX pfield?
         const double outdur = _durrand->value();
         const double amp = _amprand->value();
         const double pan = _outchans > 1 ? _panrand->value() : 1.0;
         const double pitch = getPitch();

         if (outdur >= 0.0)
            _voices[voice]->startGrain(bufoutstart, startphase, outdur, amp,
                                                                pitch, pan);
      }

      const int outjitter = (_maxoutjitter == 0.0) ? 0
                                          : int(_outrand->value() * _srate);
      // NB: outjitter can be negative, so we must ensure that _nextoutstart
      // will not be less than the next _outframecount.
      _nextoutstart = _outframecount + _hop + outjitter;
      if (_nextoutstart <= _outframecount)
         _nextoutstart = _outframecount + 1;
   }
}


// Called for single-frame I/O.

void SynthGrainStream::playGrains()
{
   _lastL = 0.0f;
   _lastR = 0.0f;
   for (int i = 0; i < MAX_NUM_VOICES; i++) {
      if (_voices[i]->inUse()) {
         float sigL, sigR;
         _voices[i]->next(sigL, sigR);
         _lastL += sigL;
         _lastR += sigR;
      }
   }
}


// Called for block I/O.

void SynthGrainStream::playGrains(float buffer[], const int numFrames,
   const float amp)
{
   const int count = numFrames * _outchans;
   for (int i = 0; i < count; i++)
      buffer[i] = 0.0f;

   for (int i = 0; i < MAX_NUM_VOICES; i++)
      if (_voices[i]->inUse())
         _voices[i]->next(buffer, numFrames, amp);
}


// Compute one frame of samples across all active grains.  Activate a
// new grain if it's time.

void SynthGrainStream::prepare()
{
   maybeStartGrain();
   playGrains();
   _outframecount++;
}


// Compute a block of samples and write them into <buffer>, which is
// assumed to hold <numFrames> frames of <_outchans> chans.

void SynthGrainStream::processBlock(float *buffer, const int numFrames,
   const float amp)
{
   for (int i = 0; i < numFrames; i++) {
      maybeStartGrain(i);
      _outframecount++;
   }
   playGrains(buffer, numFrames, amp);
}


