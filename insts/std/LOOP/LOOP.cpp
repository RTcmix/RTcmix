/* LOOP - An instrument which treats the input file like a sampler instrument.

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = transposition (8ve PPC)
   p5 = loop start
   p6 = loop length
   p7 = input channel (optional)
   p8 = pan (optional)

   p3 (amp), p4 (transposition), p5 and p6 (loop params), and p8 (pan) can receive updates from a table or real-time
   control source.

   If the loop start or loop length are negative, they are treated as a time in seconds.
   Doug Scott <netdscott at netscape dot net>, 8/20/13
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include "LOOP.h"          // declarations for this instrument class
#include <rt.h>
#include <rtdefs.h>

static const float kOneover_cpsoct10 = 1.0 / cpsoct(10.0);
static const int kMinLoopLen = 16;

static float getIncrement(float pchOct)
{
	return cpsoct(10.0 + octpch(pchOct)) * kOneover_cpsoct10;
}

LOOP::LOOP()
	: _in(NULL), _usesPan(false), _branch(0), _inLoc(0), _position(0), _lastInPosition(-1)
{
}

LOOP::~LOOP()
{
	delete [] _in;
}

int LOOP::calculateLoop(double *pArray)
{
	if (pArray[5] < 0.0) {
		_loopStart = (int) (0.5 + pArray[5] * SR);
	}
	else {
		_loopStart = (int) pArray[5];
	}
	
	float loopLength = (int) pArray[6];
	
	if (loopLength < 0.0) {
		loopLength = (int) (0.5 + loopLength * SR);
	}
	
	if (loopLength < kMinLoopLen) {
		loopLength = kMinLoopLen;
		rtcmix_warn("LOOP", "Loop length limited to %d frames", kMinLoopLen);
	}
	
	_loopEnd = _loopStart + loopLength - 1;
	
	return 0;
}

int LOOP::init(double p[], int n_args)
{
	if (n_args < 7)
		return die("LOOP",
				   "Usage: LOOP(start, inskip, dur, amp, trans, loopstart, looplen[, inchan, pan])");

	const float outskip = p[0];
	const float inskip = p[1];
	float dur = p[2];
	if (dur < 0.0)
		dur = -dur - inskip;

	_incr = getIncrement(p[4]);
	
	_inchan = (n_args > 7) ? (int) p[7] : 0;
	_usesPan = (n_args > 8);
	_pan = _usesPan ? p[8] : 0.5;

	if (calculateLoop(p) == -1)
		return DONT_SCHEDULE;
	
	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("LOOP", "Use mono or stereo output only.");

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	if (_inchan >= inputChannels())
		return die("LOOP", "You asked for channel %d of a %d-channel input.", _inchan, inputChannels());

	_position = inskip * SR;	// Actual starting position in the input file
	_inOffset = 0;	// Offset of first sample in _in array compared to _position.
	
	return nSamps();
}

int LOOP::configure()
{
	_in = new float [RTBUFSAMPS * inputChannels()];

	return _in ? 0 : -1;	// IMPORTANT: Return 0 on success, and -1 on failure.
}

int LOOP::doupdate()
{
	double p[9];
	update(p, 9);

	_amp = p[3];
	_incr = getIncrement(p[4]);
	_pan = _usesPan ? p[8] : 0.5;
	return calculateLoop(p);
}

int LOOP::run()
{
	const int outframes = framesToRun();
	const int inchans = inputChannels();
	const int samps = outframes * inchans;
	int loopLength = _loopEnd - _loopStart + 1;
	double end = _loopEnd + 1;

//	printf("LOOP::run() TOP: _position: %.2f\n", _position);
	
	for (int i = 0; i < outframes; ++i) {

		if (--_branch <= 0) {
			if (doupdate() == -1)
				return -1;
			loopLength = _loopEnd - _loopStart + 1;
			end = _loopEnd + 1;
			_branch = getSkip();
		}
		
		if (_position >= _lastInPosition) {
			int backskip = int(_position - _inLoc) - _inOffset;
			rtinrepos(this, -backskip, SEEK_CUR);
			rtgetin(_in, this, samps);
			_lastInPosition += (RTBUFSAMPS - backskip);
			_inOffset = int(_position);
		}
		float fLoc = (_position - _inOffset);
		_inLoc = (int) fLoc;	// read location in _in
		const float frac = fLoc - _inLoc;
		const int loc = (_inLoc * inchans) + _inchan;
		float insig = _in[loc] + (frac * (_in[loc+1] - _in[loc])) * _amp;

		float out[2];

		out[0] = insig;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0f - _pan);
			out[0] *= _pan;
		}

		rtaddout(out);

		increment();
		
		_position += _incr;
		
		if (_position >= end) {
//			printf("LOOP::run(): hit loop end: (%.2f >= %.2f).  last interp before loop: %.2f between _in[%d] and _in[%d]\n",
//				   _position, end, frac, _inLoc, _inLoc+1);
//			if (_position - _loopStart > _inLoc) {		// loop start is not contained within current _in array
			if (_inOffset > _loopStart) {		// loop start is not contained within current _in array
				rtinrepos(this, (int)_loopStart, SEEK_SET);	// set to first frame of loop
				_position -= loopLength;
				_inLoc = 0;
				_lastInPosition = int(_position - 1);			// forces rtgetin()
				_inOffset = _position;
//				printf("LOOP::run() seeking back to %d.  _position will be %.2f\n\n", (int)_loopStart, _position);
			}
			else {										// loop is within _in array
				_position -= loopLength;
//				printf("LOOP::run() hit loop end: loop within buffer.  _position will be %.2f\n\n", _position);
			}
		}
	}

//	printf("LOOP::run() EXIT: _inLoc: %d, _inOffset: %d _position: %.2f\n\n", _inLoc, _inOffset, _position);

	return framesToRun();
}

Instrument *makeLOOP()
{
	LOOP *inst = new LOOP();
	inst->set_bus_config("LOOP");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("LOOP", makeLOOP);
}
#endif

