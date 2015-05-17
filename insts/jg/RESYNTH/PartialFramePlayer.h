// Copyright (C) 2012 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.
//
// This version of PartialFramePlayer is for use in the RTcmix RESYNTH
// instrument and variants. A different version is used for MaxMSP objects.

#ifndef _PARTIALFRAMEPLAYER_H_
#define _PARTIALFRAMEPLAYER_H_

#include "PartialVoice.h"
#include "List.h"
#include "RandGen.h"
#include <math.h>	// for log

class Frame;

// midC freq is 440.0 * pow(2.0, -9.0 / 12.0) = 261.625...
#define MIDC_OFFSET (261.62556530059868 / 256.0)
#ifndef M_LN2		// log_e 2
	#define M_LN2	0.69314718055994529
#endif


class PartialFramePlayer {
public:
	PartialFramePlayer(float srate, int bufFrames, int numChans = 1);
	~PartialFramePlayer(void);

	// Must call this with values obtained from the SDIF object
	// before calling these methods: setAverageFreqs, setFrame, or
	// synthesizeBlock.
	void init(int maxSimultaneousPartials, int maxPartialID);

	// Set how many sample frames to skip between consulting envelopes.
	// This value must have already been determined by a call to
	// getControlRateMultiplier (its second argument, passed by reference).
	void controlSamps(const int samps) { _controlSamps = samps; }

	// getControlRateMultiplier() returns a value that describes the speed of
	// control rate updates in relation to audio buffer size. <bufSize> is the
	// size of the audio buffer in samples.  <controlSize> is the number of
	// samples between control updates, specified by the user. The code rounds
	// this size to the nearest valid size and passes it back by reference.
	// If the value returned, X, is greater than 0, then a control rate update
	// happens X times within a single audio buffer. The code insures that the
	// buffer size is evenly divisible by this number. If X is less than 1, then
	// X expresses the number of times to do a control rate update relative to
	// audio buffer size: 0.5 means every other buffer, 0.25 means every fourth
	// buffer, etc. If X is -1, it means the search failed, which is probably
	// impossible.
	//
	// This is a static method so that it can be called to construct a buffering
	// scheme at a level higher than this object.  For example, this is
	// necessary in RTcmix, though not in MaxMSP.
	static float getControlRateMultiplier(const int bufSize, int &controlSize);

	// Set how long a partial's amplitude ramps down to 0 once it is set to die.
	// NOTE: Must call this only after a call to controlSamps() or controlRate()!
	void interpTime(const float time)
	{
		_interpSamps = int((time * _srate / _controlSamps) + 0.5);
		if (_busyVoices) {
			PartialVoice *p = _busyVoices->head();
			while (p) {
				p->interpSamps(_interpSamps);
				p = p->next();
			}
		}
	}

	// Given an array, sized maxPartialID+1, of average frequencies (in Hz) for
	// each partial track, convert to linear octaves and store into a private
	// array. These are used when deciding whether a partial track should be
	// retuned.
	void setAverageFreqs(const float avgFreqs[])
	{
		if (avgFreqs == NULL)
			return;
		for (int i = 0; i < _numPartialIDs; i++)
			_avgFreqLinoct[i] = _octcps(avgFreqs[i]);
	}

	// If true, retune partials to the list of pitches passed to
	// setRetuneChord(), leaving alone any partials whose average frequencies
	// are not close enough to these pitches. Closeness is determined by
	// retuneSensitivity().
	void retunePartials(const bool retune) { _retunePartials = retune; }

	// List of partial pitches in oct.pc for retuning.
	// The <chord> array is consulted only during this call.
	void setRetuneChord(const float *chord, const int numPitches);

	// Transpose all the pitches in the retune chord (semitones).
	void retuneTranspose(const float transpose)
	{
		_chordTransposition = _octpch(transpose * 0.01);
	}

	// Retune if partial is within this interval (in semitones) of one of the
	// pitches in the list passed to retuneChord().
	void retuneSensitivity(const float sensitivity)
	{
		_retuneSensitivity = _octpch(sensitivity * 0.01);
	}

	// Set the extent (0-1) to which a retuned partial conforms to a close
	// target pitch in the list passed to retuneChord().
	void retuneStrength(const float strength) { _retuneStrength = strength; }

	// Scale frequencies of all current and future partials by <factor>.
	void freqScaleAll(const float factor)
	{
		if (_busyVoices) {
			PartialVoice *p = _busyVoices->head();
			while (p) {
				p->freqScale(factor);
				p = p->next();
			}
		}
		_freqScaleFactor = factor;
	}

	// Add <offset> to the frequencies of all current and future partials,
	// after scaling them.
	void freqAddAll(const float offset)
	{
		if (_busyVoices) {
			PartialVoice *p = _busyVoices->head();
			while (p) {
				p->freqAdd(offset);
				p = p->next();
			}
		}
		_freqOffset = offset;
	}

	// Change the frame of partials being played to this one. If the time of
	// of this frame is the same as the last one, nothing new will happen,
	// unless <forceUpdate> is true. Return 1 if frame changed, 0 if ignoring it.
	int setFrame(Frame *frame, bool forceUpdate=false);

	// Set seed for panning random number generator.
	void panSeed(const int seed) { _panRand->setseed(seed); }

	// Synthesize an interleaved block of sample frames.
	void synthesizeBlock(float outBuffer[], const int numFrames,
				const int numChans, const float ampScale = 1.0f);

private:
	// Various pitch conversion utilities.
	double _cpsoct(const double oct)
	{
		return pow(2.0, oct) * MIDC_OFFSET;
	}
	double _octcps(const double cps)
	{
		return log(cps / MIDC_OFFSET) / M_LN2;
	}
	double _octpch(const double pch)
	{
		const int octave = int(pch);
		const double semitone = (100.0 / 12.0) * (pch - octave);
		return octave + semitone;
	}

	// Allocate partial voices and push them onto the front of <list>,
	// which can be NULL.
	void _allocVoices(List <PartialVoice *> *list, const int numVoices);

	float _retunePartial(const int partialID, const float origFreq);

	// Prune dead voices from the active voice list, push them onto the free
	// list, and mark their entries in in the _partialIDtoVoice table as NULL.
	// When a dying partial voice completes its ramp to zero amplitude, it marks
	// itself as unused. Here we	use that to identify them.
	void _pruneVoices(void);

	// Debugging utils
	int _dumpPartialStates(void);
	int _dumpActivePartialIDs(void);
	int _dumpActivePartials(void);

	bool				_retunePartials;
	int				_bufFrames, _interpSamps;
	int				_numPartialIDs, _numChordPitches, _numChans, _controlSamps;
	float				_srate, _chordTransposition, _retuneSensitivity;
	float				_retuneStrength, _freqScaleFactor, _freqOffset;
	float				*_wavetable, *_avgFreqLinoct, *_chordPitches;
	double			_prevTime;
	PartialVoice	**_partialIDtoVoice;
	RandGen			*_panRand;
	List <PartialVoice *> *_freeVoices, *_busyVoices;
};

#endif // _PARTIALFRAMEPLAYER_H_
