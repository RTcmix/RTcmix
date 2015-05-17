// Copyright (C) 2012 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#ifndef _PARTIALVOICE_H_
#define _PARTIALVOICE_H_

#include "Sine.h"
#include "Interpolator.h"

class PartialFramePlayer;

// this is shared by PartialVoice and PartialFramePlayer classes
typedef enum {
	PartialStateUnused = 0,
	PartialStatePrevAlive,
	PartialStateAlive,
	PartialStateDying
} PartialState;

class PartialVoice {
public:
	PartialVoice(PartialFramePlayer *partialFramePlayer, float srate,
		float wavetable[], int tablelen);
	~PartialVoice(void);

	void interpSamps(const int numSamples)
	{
		_freqInterp->samps(numSamples);
		_ampInterp->samps(numSamples);
	}

	// NB: ID is -1 if voice is currently unused
	void id(const int id) { _id = id; }
	int id(void) const { return _id; }

	void state(const PartialState state) { _state = state; }
	PartialState state(void) const { return _state; }

	void init(const float freq, const float amp)
	{
		_freqInterp->init(freq);
		_ampInterp->init(amp);
	}

	// Begin amplitude ramp down to zero.
	void die(void)
	{
		_ampInterp->target(0.0);
		_state = PartialStateDying;
	}

	void freq(const float freq) { _freqInterp->target(freq); }
	float freq(void) const { return _freqInterp->target(); }
	void freqScale(const float factor) { _freqScaleFactor = factor; }
	void freqAdd(const float offset) { _freqOffset = offset; }
	void amp(const float amp) { _ampInterp->target(amp); }
	void ampScale(const float factor) { _ampScaleFactor = factor; }
	void phase(const float phase) { _oscil->setPhaseRadians(phase); }

	void pan(const float pan)
	{
		_pan = pan;
		_chan = int(pan);
	}
	float pan(void) const { return _pan; }
	void chan(const int chan) { _chan = chan; }
	int chan(void) const { return _chan; }

	// Synthesize a block of interleaved sample frames. This will be called 
	// from a high-priority audio thread. Return 1 if partial is now dead,
	// 0 if still alive, and -1 if the partial is unused (shouldn't happen).
	int synthesizeBlock(float outBuffer[], const int numFrames,
				const int numChans, const float ampScale);

//FIXME: these should be accessible only by PartialFramePlayer and this obj
	PartialVoice * next(void) const { return _next; }
	void next(PartialVoice * voice) { _next = voice; }

private:
	int _updateParams(float &amp);

	PartialVoice		*_next;	// link to next voice, or NULL if list tail
	int					_id, _chan;
	float					_srate, _freqScaleFactor, _freqOffset;
	float					_ampScaleFactor, _pan;
	PartialState		_state;
	Sine					*_oscil;
	Interpolator		*_freqInterp, *_ampInterp;
	PartialFramePlayer *_partialFramePlayer;
};

#endif // _PARTIALVOICE_H_
