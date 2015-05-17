// Copyright (C) 2012 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include "PartialFramePlayer.h"
#include "Frame.h"
#include <math.h>
#include <stdio.h>
#include <float.h>	// for FLT_MAX
//#define NDEBUG
#include <assert.h>

//#define DEBUG1
//#define DEBUG2
//#define DEBUG3
//#define DEBUG4
// debugging tip: break into gdb on i386: asm("int $3");

#ifndef M_PI
	#define M_PI	3.14159265358979323846264338327950288
#endif
#ifndef TWOPI
	#define TWOPI	(M_PI * 2)
#endif

const int kWavetableSize = 32768;

// How many voices to allocate at once if we run out.
const int kExtraVoiceAlloc = 10;

// A PartialFramePlayer contains a list of PartialVoice objects, each of which
// can play a partial track. An SDIF file presents these tracks as threads
// linking a number of consecutive frames, associated by a "partial index,"
// which we call "ID" to avoid confusion with array indices.  For example, ten
// consecutive frames might each have an entry for a partial with ID 230. The
// common ID is the only way for us to tell that the partial entries belong
// together, because the birth and death of partials causes the ordering of
// partial tracks to shift from frame to frame. Partial IDs range from 0 to
// <maxPartialID>, obtained by analyzing the SDIF file.

PartialFramePlayer::PartialFramePlayer(float srate, int bufFrames, int numChans)
	: _retunePartials(false), _bufFrames(bufFrames), _interpSamps(100),
	  _numChordPitches(0), _numChans(numChans), _controlSamps(bufFrames), 
	  _srate(srate), _chordTransposition(0.0), _retuneSensitivity(0.0),
	  _retuneStrength(0.0), _freqScaleFactor(1.0), _freqOffset(0.0),
	  _avgFreqLinoct(NULL), _chordPitches(NULL), _prevTime(-1.0),
	  _partialIDtoVoice(NULL), _freeVoices(NULL), _busyVoices(NULL)
{
	// Make sine table to be shared among all oscillators.
	_wavetable = new float [kWavetableSize];
	for (int i = 0; i < kWavetableSize; i++) {
		double val = TWOPI * (double) i / kWavetableSize;
		_wavetable[i] = sin(val);
	}

	const double min = 0.0;
	const double max = (_numChans == 2) ? 1.0 : _numChans - 0.000001;
	_panRand = new LinearRandom(min, max);
}

PartialFramePlayer::~PartialFramePlayer(void)
{
	if (_freeVoices) {
		_freeVoices->deleteElements();
		delete _freeVoices;
	}
	if (_busyVoices) {
		_busyVoices->deleteElements();
		delete _busyVoices;
	}
	delete [] _partialIDtoVoice;
	delete [] _avgFreqLinoct;
	delete [] _chordPitches;
	delete [] _wavetable;
	delete _panRand;
}

// We make an initial allocation of partial voices and expand it later if
// necessary. Because we have random access to the frames in the SDIF file, as
// well as arbitrarily long decay times for the partials, it is impossible to
// predict how many partials are required for a given run. But in nearly every
// case, the number of partial voices is smaller (potentially much smaller)
// than the number of partial tracks (and therefore <maxPartialID>) in an SDIF
// file. To avoid as many data structure searches as possible, we maintain an
// array (_partialIDtoVoice) that maps partial indices (from the SDIF file) to
// PartialVoice objects.  As partials are born and die, the partial voices are
// listed as alive, dying, or unused, within PartialVoice.

void PartialFramePlayer::init(int maxSimultaneousPartials, int maxPartialID)
{
	if (_freeVoices) {
		_freeVoices->deleteElements();
		delete _freeVoices;
	}
	if (_busyVoices) {
		_busyVoices->deleteElements();
		delete _busyVoices;
	}

	// Create two singly-linked lists to hold used and unused voices.
	// Make an initial allocation of partial voices and push them onto the
	// front of the free list.
	_freeVoices = new List<PartialVoice *>();
	_allocVoices(_freeVoices, maxSimultaneousPartials * 2);
	_busyVoices = new List<PartialVoice *>();

	// Make a <_partialIDtoVoice> table that maps a partial ID (stored in
	// the SDIF file) to a PartialVoice object, so that we can quickly connect
	// partial data from frame to frame. Also, make a table to hold average
	// frequency in linear octaves of each partial track.
	_numPartialIDs = maxPartialID + 1;
	_partialIDtoVoice = new PartialVoice * [_numPartialIDs];
	_avgFreqLinoct = new float [_numPartialIDs];
	for (int i = 0; i < _numPartialIDs; i++) {
		_partialIDtoVoice[i] = NULL;	// mark as unused
		_avgFreqLinoct[i] = 0.0f;
	}
}

void PartialFramePlayer::_allocVoices(List <PartialVoice *> *list, const int numVoices)
{
	for (int i = 0; i < numVoices; i++) {
		PartialVoice *voice = new PartialVoice(this, _srate, _wavetable, kWavetableSize);
		list->pushFront(voice);
	}
}

float PartialFramePlayer::getControlRateMultiplier(const int bufSize, int &controlSize)
{
	float result = -1.0f;
	if (controlSize < 1)
		return result;
	if (controlSize >= bufSize) {		// get nearest multiple
		int multiple = 2;
		int prevSize = bufSize;
		while (1) {
			const int newSize = bufSize * multiple;
			if (controlSize < newSize) {
				const int diff1 = controlSize - prevSize;
				const int diff2 = newSize - controlSize;
				if (diff1 < diff2)
					result = float(multiple - 1);
				else
					result = float(multiple);
				result = 1.0f / result;
				break;
			}
			prevSize = newSize;
			multiple += 1;
		}
	}
	else {	// get nearest divisor
		const int greatestDivisor = bufSize / 2;
		int divisor = 2;
		int prevDivisor = 1;
		int prevSize = bufSize;
		while (divisor <= greatestDivisor) {
			if ((bufSize % divisor) == 0) {
				const int newSize = bufSize / divisor;
				if (controlSize >= newSize) {
					const int diff1 = prevSize - controlSize;
					const int diff2 = controlSize - newSize;
					if (diff1 < diff2)
						result = float(prevDivisor);
					else
						result = float(divisor);
					break;
				}
				prevDivisor = divisor;
				prevSize = newSize;
			}
			divisor += 1;
		}
	}
	controlSize = int(bufSize / result);
	return result;
}

void PartialFramePlayer::setRetuneChord(const float *chord, const int numPitches)
{
	_numChordPitches = numPitches;
	delete [] _chordPitches;
	if (_numChordPitches == 0)
		_chordPitches = NULL;
	else {
		_chordPitches = new float [_numChordPitches];
		for (int i = 0; i < _numChordPitches; i++)
			_chordPitches[i] = _octpch(chord[i]);
	}
}

// Retune a partial breakpoint.
//FIXME:
// This applies to each *frame*, not an entire track. If we just sit on
// one frame, then we can't change the freq & amp if we do this here. Well, not
// quite true: if we push new freq/amp settings into PartialFramePlayer, then at
// the same time we could force sending a new frame, which would trigger the
// call to retunePartial (etc) from setFrame.

float PartialFramePlayer::_retunePartial(const int partialID, const float origFreq)
{
	if (_retunePartials && _chordPitches) {
		float minInterval = FLT_MAX;
		int chordIndex = -1;
		for (int i = 0; i < _numChordPitches; i++) {
			float targetPitch = _chordPitches[i] + _chordTransposition;
			float interval = fabs(_avgFreqLinoct[partialID] - targetPitch);
			if (interval < minInterval) {
				minInterval = interval;
				chordIndex = i;
			}
		}
		if (chordIndex >= 0 && minInterval <= _retuneSensitivity) {
			// If so, retune this partial breakpoint.
			float targetPitch = _chordPitches[chordIndex] + _chordTransposition;
			// Move the partial breakpoint freq closer to the target pitch,
			// according to _retuneStrength.
			const float origPitch = _octcps(origFreq);	//FIXME: ??could be cached
			const float diff = targetPitch - origPitch;
			return _cpsoct(origPitch + (diff * _retuneStrength));
		}
	}
	return origFreq;
}

// Accept a frame from the SDIF file, distribute the data to the appropriate
// partial track voices, and perform various bookkeeping tasks to manage the
// birth and death of partials. Do not do all this if <frame> is the same as
// the one received in the last call to this method, unless <forceUpdate> is
// true.
//
// NOTE: If interpTime spans more than one frame's duration, then a partial
// will be in its dying state (ramp to zero amplitude) across more than one
// frame.

int PartialFramePlayer::setFrame(Frame *frame, bool forceUpdate)
{
#ifdef DEBUG1
	int continueCount = 0;
	int newCount = 0;
	int newDyingCount = 0;
	int prevDyingCount = 0;
#endif
	const double thisTime = frame->time();
	if (thisTime == _prevTime && !forceUpdate) {
#ifdef DEBUG3
		printf("setFrame: noop return, frame time=%f\n", thisTime);
#endif
		return 0;
	}
	PartialNode **partials = frame->partials();
	const int numPartials = frame->numPartials();
	List <PartialVoice *> newVoices;
	for (int i = 0; i < numPartials; i++) {
		// Check if partial is already playing in a voice.
		PartialNode *partial = partials[i];
		const int partialID = partial->id();
		float freq = _retunePartial(partialID, partial->freq());
		PartialVoice *voice = _partialIDtoVoice[partialID];
		if (voice == NULL) {		// partial didn't exist in previous frame
			voice = _freeVoices->popFront();
			if (voice == NULL) {
				_allocVoices(_freeVoices, kExtraVoiceAlloc);
				voice = _freeVoices->popFront();
			}
			voice->init(freq, 0.0);
			voice->id(partialID);
			voice->interpSamps(_interpSamps);
			// set initial freq, amp, and phase only when starting a new
			// partial track
#ifdef NOMORE // wouldn't the phase have to be changed if we change freq?
//FIXME: should set phase to sdif val, or at least to zero
			voice->phase(partial->phase());
#endif
			voice->phase(0.0);
			voice->freqScale(_freqScaleFactor);
			voice->freqAdd(_freqOffset);
			if (_numChans > 1)
				voice->pan(_panRand->value());
			voice->state(PartialStatePrevAlive);
			_partialIDtoVoice[partialID] = voice;
			newVoices.pushFront(voice);
#ifdef DEBUG1
			printf("n%d ", voice->id());
			newCount++;
#endif
		}
		else {
			voice->state(PartialStateAlive);
			// NB: voice is already in _busyVoices
#ifdef DEBUG1
			continueCount++;
#endif
		}
		voice->freq(freq);
		voice->amp(partial->amp());
	}
#ifdef DEBUG1
	if (newCount)
		printf("\n");
#endif
//_busyVoices->sanityCheck();

	PartialVoice *p = _busyVoices->head();
	while (p) {
		// For any partials that were alive in the previous frame, but are not
		// alive in the current frame, put them into dying state, which starts an
		// amplitude ramp to zero.  If state is PartialStatePrevAlive, it's not
		// in the current frame; if it were, it would've been set to
		// PartialStateAlive above.
		if (p->state() == PartialStatePrevAlive) {
//FIXME: set freq again, in case retuning has changed -- NEEDS TESTING
			const float newfreq = _retunePartial(p->id(), p->freq());
			p->freq(newfreq);
			p->die();			// sets state to PartialStateDying
#ifdef DEBUG1
			newDyingCount++;
#endif
		}
		// Mark all currently alive partials as previously sounding ones,
		// for use when next frame arrives.
		else if (p->state() == PartialStateAlive) {
			p->state(PartialStatePrevAlive);
		}
		// Else they were set to dying in a previous frame, but have not died yet.
		else {
//FIXME: set freq again, in case retuning has changed -- NEEDS TESTING
			const float newfreq = _retunePartial(p->id(), p->freq());
			p->freq(newfreq);
#ifdef DEBUG1
			prevDyingCount++;
#endif
		}
		p = p->next();
	}

	_busyVoices->appendList(&newVoices);

#ifdef DEBUG1
	printf("setFrame: time=%f, partials=%d: contin=%d, born=%d, newdying=%d prevdying=%d\n",
		thisTime, numPartials, continueCount, newCount, newDyingCount, prevDyingCount);
#endif
#ifdef DEBUG2
	(void) _dumpPartialStates();
#endif
#ifdef DEBUG4
	const int count = _dumpActivePartials();
	if (count != (numPartials + newDyingCount + prevDyingCount))
		printf("****** WARNING: partial leak!\n");
#endif

	_prevTime = thisTime;
	return 1;
}

void PartialFramePlayer::_pruneVoices(void)
{
//if (_busyVoices->sanityCheck(false)) printf("...at beginning of _pruneVoices\n");
	PartialVoice *p = _busyVoices->head();
	PartialVoice *prev = NULL;
	while (p) {
		if (p->state() == PartialStateUnused) {
//printf("partial unused: %p (id=%d)\n", p, p->id());
			const int id = p->id();
			assert(id >= 0 && id < _numPartialIDs);
			_partialIDtoVoice[id] = NULL;
			PartialVoice *tmp = p;
			p = p->next();
//printf("removing %p from list %p, prev=%p prev->next=%p (p=%p)\n", tmp, _busyVoices, prev, prev ? prev->next() : NULL, p);
			_busyVoices->removeElement(tmp, prev);
//_busyVoices->dump();
//if (_busyVoices->sanityCheck(false)) printf("...before pushFront\n");
			_freeVoices->pushFront(tmp);
//if (_busyVoices->sanityCheck(false)) { printf("...after pushFront\n"); _busyVoices->dump(true); _freeVoices->dump(); }
		}
		else {
			prev = p;
			p = p->next();
		}
	}
//if (_busyVoices->sanityCheck(false)) printf("...at end of _pruneVoices\n");
//if (_freeVoices->sanityCheck(false)) printf("...at end of _pruneVoices\n");
}

void PartialFramePlayer::synthesizeBlock(
	float outBuffer[],		// interleaved buffer to fill, allocated by caller
	const int numFrames,		// number of frames in outBuffer
	const int numChans,		// number of channels in outBuffer
	const float ampScale)	// scale all amplitudes by this amount
{
	// Zero the buffer so that we can accumulate samples into it.
	const int numSamps = numFrames * numChans;
	for (int i = 0; i < numSamps; i++)
		outBuffer[i] = 0.0f;

	PartialVoice *p = _busyVoices->head();
	while (p) {
		p->synthesizeBlock(outBuffer, numFrames, numChans, ampScale);
		p = p->next();
	}
	_pruneVoices();
}

// Print partial states in linked list order. Return number of partials.
int PartialFramePlayer::_dumpPartialStates(void)
{
	printf("partialState -----------------------------------------------------\n");
	int count = 0;
	PartialVoice *p = _busyVoices->head();
	while (p) {
		printf("%d ", p->state());
		p = p->next();
		count++;
	}
	printf("\n%d partials\n", count);
	return count;
}

// Print partial IDs in linked list order. Return number of partials.
int PartialFramePlayer::_dumpActivePartialIDs(void)
{
	printf("active partial IDs -----------------------------------------------\n");
	int count = 0;
	PartialVoice *p = _busyVoices->head();
	while (p) {
		printf("%d ", p->id());
		p = p->next();
		count++;
	}
	printf("\n%d partials\n", count);
	return count;
}

// Print partial pointers in linked list order. Return number of partials.
int PartialFramePlayer::_dumpActivePartials(void)
{
	printf("active partial pointers ------------------------------------------\n");
	int count = 0;
	PartialVoice *p = _busyVoices->head();
	while (p) {
		printf("%p ", p);
		p = p->next();
		count++;
	}
	printf("\n%d partials\n", count);
	return count;
}

