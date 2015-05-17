// Copyright (C) 2012 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include "PartialFramePlayer.h"
#include "PartialVoice.h"

// Interpolation doesn't seem to be necessary if we use a large wave table.
//#define USE_INTERPOLATING_OSCIL

PartialVoice::PartialVoice(PartialFramePlayer *partialFramePlayer, float srate,
			float wavetable[], int tablelen)
	: _id(-1), _chan(0), _srate(srate),
	  _freqScaleFactor(1.0), _freqOffset(0.0), _ampScaleFactor(1.0),
	  _pan(0.0), _state(PartialStateUnused),
	  _partialFramePlayer(partialFramePlayer)
{
	_oscil = new Sine(_srate, 440.0, wavetable, tablelen);
	_freqInterp = new Interpolator();
	_ampInterp = new Interpolator();
}

PartialVoice::~PartialVoice(void)
{
	delete _oscil;
	delete _freqInterp;
	delete _ampInterp;
}

int PartialVoice::_updateParams(float &amp)
{
	if (_state == PartialStateUnused)
		return -1;

	// Update parameters refreshed from control sources since last time.
	amp = _ampInterp->next();
	if (amp == 0.0f && _state == PartialStateDying) {
		// partial is now dead
		_state = PartialStateUnused;
		return 1;
	}
	amp *= _ampScaleFactor;
	float freq = (_freqInterp->next() * _freqScaleFactor) + _freqOffset;
	if (freq < 10.0f)	// our oscil cannot handle negative freqs,
		freq = 10.0f;	// and we don't want DC either
	_oscil->setfreq(freq);

	return 0;
}

int PartialVoice::synthesizeBlock(
	float outBuffer[],		// cleared, interleaved buffer to accumulate to
	const int numFrames,		// number of frames in outBuffer
	const int numChans,		// number of channels in outBuffer
	const float ampScale)	// scale all amplitudes by this amount
{
	float amp;
	const int result = _updateParams(amp);
	if (result != 0)
		return result;
	amp *= ampScale;

	// Add new audio to the output buffer.
	if (numChans == 1) {
		for (int i = 0; i < numFrames; i++) {
#ifdef USE_INTERPOLATING_OSCIL
			outBuffer[i] += _oscil->nexti() * amp;
#else
			outBuffer[i] += _oscil->next() * amp;
#endif
		}
	}
	else if (numChans == 2) {
		// Pan using simple linear panning law. NB: This is more expensive than
		// the quad point-source version, even with just linear panning.
		float *out = outBuffer;
		for (int i = 0; i < numFrames; i++) {
#ifdef USE_INTERPOLATING_OSCIL
			float sigL = _oscil->nexti() * amp;
#else
			float sigL = _oscil->next() * amp;
#endif
			float sigR = sigL * (1.0f - _pan);
			sigL *= _pan;
			*out++ += sigL;
			*out++ += sigR;
		}
	}
	else {
		// Multichannel pan using point-source locations for an arbitrary number
		// of chans.
		float *out = outBuffer;
		for (int i = 0; i < numFrames; i++) {
#ifdef USE_INTERPOLATING_OSCIL
			float sig = _oscil->nexti() * amp;
#else
			float sig = _oscil->next() * amp;
#endif
			out[_chan] += sig;
			out += numChans;
		}
	}
	return 0;
}

