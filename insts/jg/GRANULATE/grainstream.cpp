// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <ugens.h>   // for octpch and ampdb
#include <Ougens.h>  // for Ooscil
#include "grainstream.h"
#include "grainvoice.h"
//#define NDEBUG       // disable asserts
#include <assert.h>

#define DEBUG 0

#define ALL_VOICES_IN_USE  -1

#define COUNT_VOICES

inline int _clamp(const int min, const int val, const int max)
{
   if (val < min)
      return min;
   if (val > max)
      return max;
   return val;
}


// NOTE: We don't own the table memory.
GrainStream::GrainStream(const float srate, double *inputTable, int tableLen,
   const int numInChans, const int numOutChans, const bool preserveGrainDur,
   const int seed, const bool use3rdOrderInterp)
   : _srate(srate), _inputtab(inputTable), _inputframes(tableLen / numInChans),
     _outchans(numOutChans), _inchan(0), _winstart(-1), _winend(_inputframes),
     _wrap(true), _inhop(0), _outhop(0), _maxinjitter(0.0), _maxoutjitter(0.0),
     _transp(0.0), _maxtranspjitter(0.0), _transptab(NULL), _transplen(0),
     _outframecount(0), _nextinstart(0), _nextoutstart(0), _travrate(1.0),
     _lasttravrate(1.0), _lastinskip(DBL_MAX), _lastL(0.0f), _lastR(0.0f)
{
   for (int i = 0; i < MAX_NUM_VOICES; i++)
      _voices[i] = new GrainVoice(_srate, _inputtab, _inputframes, numInChans,
                            numOutChans, preserveGrainDur, use3rdOrderInterp);
   _inrand = new LinearRandom(0.0, 1.0, seed);
   _outrand = new LinearRandom(0.0, 1.0, seed * 2);
   _durrand = new LinearRandom(0.0, 1.0, seed * 3);
   _amprand = new LinearRandom(0.0, 1.0, seed * 4);
   _transprand = new LinearRandom(0.0, 1.0, seed * 5);
   _panrand = new LinearRandom(0.0, 1.0, seed * 6);
#ifdef COUNT_VOICES
   _maxvoice = -1;
#endif
}


GrainStream::~GrainStream()
{
   for (int i = 0; i < MAX_NUM_VOICES; i++)
      delete _voices[i];
   delete [] _transptab;
   delete _inrand;
   delete _outrand;
   delete _durrand;
   delete _amprand;
   delete _transprand;
   delete _panrand;
#ifdef COUNT_VOICES
   rtcmix_advise("GRANULATE", "Used %d voices", _maxvoice + 1);
#endif
}


// NOTE: We don't own the table memory.
// Call this before ever calling prepare or processBlock.

void GrainStream::setGrainEnvelopeTable(double *table, int length)
{
   for (int i = 0; i < MAX_NUM_VOICES; i++)
      _voices[i]->setGrainEnvelopeTable(table, length);
}


// Set inskip, overriding current position, which results from the traversal
// rate.  It's so important not to do this if the requested inskip hasn't
// changed that we do that checking here, rather than asking the caller to.
// Call setInskip just after calling setWindow.

void GrainStream::setInskip(const double inskip)
{
   if (inskip != _lastinskip) {
      _nextinstart = int((inskip * _srate) + 0.5);
      _nextinstart = _clamp(_winstart, _nextinstart, _winend);
      _lastinskip = inskip;
   }
}


// Set input start and end point in frames.  If this is the first time we're
// called, force next call to prepare or processBlock to start playing at
// start point.  Otherwise, we'll come back to new start point at wraparound.

void GrainStream::setWindow(const double start, const double end)
{
   const bool firsttime = (_winstart == -1);

   _winstart = int((start * _srate) + 0.5);
   _winend = int((end * _srate) + 0.5);
   if (_winend < _winstart) {
      int tmp = _winstart;
      _winstart = _winend;
      _winend = tmp;
   }
   if (_winstart < 0)
      _winstart = 0;
   if (firsttime)
      _nextinstart = _winstart;

   // NOTE: _winend is the last frame we use (not one past the last frame)
   if (_winend >= _inputframes)
      _winend = _inputframes - 1;
}


// Set both the input traversal rate and the output grain hop.  Here are some
// possible values for <rate>, along with their interpretation.
//
//    0     no movement
//    1     move forward at normal rate (i.e., as fast as we hop through output)
//    2.5   move forward at a rate that is 2.5 times normal
//    -1    move backward at normal rate
//
// <hop> is the number of seconds to skip on the output before starting a
// new grain.  We add jitter to this amount in maybeStartGrain().

void GrainStream::setTraversalRateAndGrainHop(const double rate,
   const double hop)
{
   const double hopframes = hop * _srate;
   _outhop = int(hopframes + 0.5);
   if (rate < 0.0)
      _inhop = int((hopframes * rate) - 0.5);
   else
      _inhop = int((hopframes * rate) + 0.5);
   _travrate = rate;
#if DEBUG > 3
   printf("inhop=%d, outhop=%d\n", _inhop, _outhop);
#endif
}


void GrainStream::setGrainTranspositionCollection(double *table, int length)
{
   delete [] _transptab;
   if (table) {
      _transptab = new double [length];
      for (int i = 0; i < length; i++)
         _transptab[i] = octpch(table[i]);
      _transplen = length;
      _transprand->setmin(0.0);
   }
   else {
      _transptab = NULL;
      _transplen = 0;
   }
}


// Given _transp and (possibly) _transptab, both in linear octaves, return
// grain transposition value in linear octaves.

const double GrainStream::getTransposition()
{
   double transp = _transp;
   const double transpjitter = (_maxtranspjitter == 0.0) ? 0.0
                                             : _transprand->value();
   if (_transptab) {
      // Constrain <transpjitter> to nearest member of transposition collection.
      double min = DBL_MAX;
      int closest = 0;
      for (int i = 0; i < _transplen; i++) {
         const double proximity = fabs(_transptab[i] - transpjitter);
         if (proximity < min) {
            min = proximity;
            closest = i;
         }
#if DEBUG > 2
         printf("transtab[%d]=%f, jitter=%f, proximity=%f, min=%f\n",
                           i, _transptab[i], transpjitter, proximity, min);
#endif
      }
      transp += _transptab[closest];
#if DEBUG > 1
      printf("transpcoll chosen: %f (linoct) at index %d\n",
                                             _transptab[closest], closest);
#endif
   }
   else {
      transp += transpjitter;
#if DEBUG > 1
      printf("transp (linoct): %f\n", transp);
#endif
   }

   return transp;
}


// Return index of first freq grain voice, or -1 if none are free.
const int GrainStream::firstFreeVoice()
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

bool GrainStream::maybeStartGrain(const int bufoutstart)
{
   bool keepgoing = true;

   if (_outframecount >= _nextoutstart) {    // time to start another grain

      bool forwards = (_travrate >= 0.0);

      // When grain traversal changes sign, we need to adjust _nextinstart,
      // because this is interpreted differently for the two directions.
      // When moving forward, it's the lowest index we read for the grain;
      // when moving backward, it's the highest.
      int inoffset = 0;
      if (_travrate != _lasttravrate) {
         if (_travrate < 0.0 && _lasttravrate >= 0.0)
            inoffset = 1;     // add grain duration to _nextinstart below
         else if (_lasttravrate <= 0.0 && _travrate > 0.0)
            inoffset = -1;    // subtract grain duration from _nextinstart below
         _lasttravrate = _travrate;
      }

      const int voice = firstFreeVoice();
      if (voice != ALL_VOICES_IN_USE) {
#ifdef COUNT_VOICES
         if (voice > _maxvoice)
            _maxvoice = voice;
#endif
         const double outdur = _durrand->value();

         // Try to prevent glitch when traversal rate crosses zero.
         if (inoffset != 0) {
            inoffset *= int((outdur * _srate) + 0.5);
            _nextinstart += inoffset;
            _nextinstart = _clamp(0, _nextinstart, _inputframes - 1);
         }

         const double amp = _amprand->value();
         const double pan = _outchans > 1 ? _panrand->value() : 1.0;
         const double transp = getTransposition();

         if (outdur >= 0.0)
            _voices[voice]->startGrain(bufoutstart, _nextinstart, outdur,
                                       _inchan, amp, transp, pan, forwards);
      }
      const int injitter = (_maxinjitter == 0.0) ? 0
                                          : int(_inrand->value() * _srate);
      _nextinstart += _inhop + injitter;

      // NB: injitter and _inhop can be negative.
      if (forwards) {
         if (_nextinstart < _winstart)
            _nextinstart = _winstart;

         const int overshoot = _nextinstart - _winend;
         if (overshoot >= 0) {
            if (_wrap) {
               _nextinstart = _winstart + overshoot;
               if (_nextinstart > _winend)
                  _nextinstart = _winend;
            }
            else
               keepgoing = false;
#if DEBUG > 0
            printf("wrap to beginning... nextinstart=%d, overshoot=%d\n",
                                                   _nextinstart, overshoot);
#endif
         }
      }
      else {
         if (_nextinstart > _winend)
            _nextinstart = _winend;

         const int overshoot = _winstart - _nextinstart;
         if (overshoot >= 0) {
            if (_wrap) {
               _nextinstart = _winend - overshoot;
               if (_nextinstart < _winstart)
                  _nextinstart = _winstart;
            }
            else
               keepgoing = false;
#if DEBUG > 0
            printf("wrap to ending... nextinstart=%d, overshoot=%d\n",
                                                   _nextinstart, overshoot);
#endif
         }
      }

      const int outjitter = (_maxoutjitter == 0.0) ? 0
                                          : int(_outrand->value() * _srate);
      // NB: outjitter can be negative, so we must ensure that _nextoutstart
      // will not be less than the next _outframecount.
      _nextoutstart = _outframecount + _outhop + outjitter;
      if (_nextoutstart <= _outframecount)
         _nextoutstart = _outframecount + 1;
   }

   return keepgoing;
}


// Called for single-frame I/O.

void GrainStream::playGrains()
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

void GrainStream::playGrains(float buffer[], const int numFrames,
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
// new grain if it's time.  Return false if caller should terminate
// prematurely, due to running out of input when not using wraparound mode;
// otherwise return true.

bool GrainStream::prepare()
{
   bool keepgoing = maybeStartGrain();
   playGrains();
   _outframecount++;

   return keepgoing;
}


// Compute a block of samples and write them into <buffer>, which is
// assumed to hold <numFrames> frames of <_outchans> chans.

bool GrainStream::processBlock(float *buffer, const int numFrames,
   const float amp)
{
   bool keepgoing = true;
   for (int i = 0; i < numFrames; i++) {
      keepgoing = maybeStartGrain(i);
      _outframecount++;
      if (!keepgoing)
         break;
   }
   playGrains(buffer, numFrames, amp);
   return keepgoing;
}


