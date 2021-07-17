// Copyright (C) 2012 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

/* RESYNTH - sinusoidal resynthesis of an SDIF file

      p0  = output start time
      p1  = duration
    * p2  = amplitude multiplier
      p3  = SDIF filename
      p4  = SDIF inskip [not yet supported]
    * p5  = SDIF time
      p6  = interpolation ramp time (seconds)
    * p7  = min frequency: omit partial tracks that go below this
    * p8  = max frequency: omit partial tracks that go above this
    * p9  = amp threshold (dBFS, e.g., -60): omit partial tracks whose max
            amp is below this
    * p10 = scale the frequencies of all partials by this factor
    * p11 = add this offset to the frequencies of all partials (after scale)
      p12 = table holding pitches for partial retuning, or zero
    * p13 = transpose all pitches in retuning table (semitones)
    * p14 = retune sensitivity (semitones)
    * p15 = retune strength (0-1)
      p16 = integer seed for panning random number generator used when output
            channels are greater than one (as given to bus_config)

   NOTES

   1. Pfields marked with '*' can receive updates from a table or real-time
      control source.

   2. The control rate (set with control_rate) can have a big effect on the
      results.

   John Gibson, 12/12/12
*/
#include <stdio.h>
#include <stdlib.h>
#include <float.h>	// for FLT_MAX
#include <ugens.h>
#include <PField.h>
#include "RESYNTH.h"
#include <rt.h>
#include <rtdefs.h>
#include "SDIFfile.h"
#include "PartialFramePlayer.h"
#include "Frame.h"

//#define DEBUG

RESYNTH::RESYNTH()
	: _bufFrames(0), _minFreq(-1.0), _maxFreq(-1.0), _ampThresh(FLT_MAX),
	  _freqScaleFactor(1.0), _freqOffset(0.0), _out(NULL),
	  _frame(NULL), _sdif(NULL)
{
}

RESYNTH::~RESYNTH()
{
	delete [] _out;
	delete _player;
	delete _sdif;
}

int RESYNTH::usage() const
{
	return die("RESYNTH",
	           "Usage: RESYNTH(start, dur, ampMult, SDIFfileName, "
	           "SDIFinskip, SDIFtime, interpRampTime, minFreq, maxFreq, "
              "ampThresh, freqScale, freqAdd, retuneTable, retuneTranspose, "
              "retuneSensitivity, retuneStrength, panSeed");
}

int RESYNTH::init(double p[], int n_args)
{
	if (n_args < 17)
		return usage();
	_nargs = n_args;

	const float outskip = p[0];
	const float dur = p[1];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() < 1)
		return die("RESYNTH", "Must have at least one channel.");

	_sdif = new SDIFfile();

//FIXME: no way yet to specify an inskip on the SDIF file
	const PField &field = getPField(3);
	const char *str = field.stringValue(0);
	const int result = _sdif->readFile(str);
	if (result != 0)
		return die("RESYNTH", "Can't open SDIF file \"%s\".", str);
	_sdif->analyze();
	const float *avgFreqs = _sdif->getAverageFreqs();

	rtcmix_advise("RESYNTH", "SDIF file: \"%s\"", str);
	rtcmix_advise("RESYNTH", "numframes=%d, maxindex=%d, maxsimulpartials=%d\n",
			_sdif->numFrames(), _sdif->maxPartialID(),
			_sdif->maxSimultaneousPartials());

	// NOTE: can't use getSkip() here, because Instrument::_skip is 0 now.
	int controlSamps = int(SR / resetval);
	float mult = PartialFramePlayer::getControlRateMultiplier(RTBUFSAMPS, controlSamps);
	if (mult == -1.0)	// don't think this can happen
		return die("RESYNTH", "Can't set buffer and control sizes.");
	if (mult <= 1.0) {
		_audioBufsPerControl = int(1.0 / mult);
		_controlCount = 1;		// force update on first audio buffer
		_bufFrames = RTBUFSAMPS;
	}
	else {
		_controlMultiplier = int(mult);
		_bufFrames = RTBUFSAMPS / _controlMultiplier;
	}
//printf("RTBUFSAMPS=%d, _bufFrames=%d, controlSamps=%d, _controlMultiplier=%d\n", RTBUFSAMPS, _bufFrames, controlSamps, _controlMultiplier);
	_player = new PartialFramePlayer(SR, _bufFrames, outputChannels());
	_player->init(_sdif->maxSimultaneousPartials(), _sdif->maxPartialID());
	_player->controlSamps(_bufFrames);	// 1:1 ratio btw audio bufs and ctrl
	_player->interpTime(p[6]);
	_player->setAverageFreqs(avgFreqs);
	_player->panSeed(int(p[16]));

   int retunetablen;
   double *retunetable = (double *) getPFieldTable(12, &retunetablen);
   if (retunetable) {
	   // BGGx wwARG!
		//float tmp[retunetablen];
	   float *tmp = new float[retunetable];
		for (int i = 0; i < retunetablen; i++)
			tmp[i] = float(retunetable[i]);
		_player->setRetuneChord(tmp, retunetablen);
		_player->retunePartials(true);
	}

	return nSamps();
}

void RESYNTH::_doupdate()
{
	double p[17];
	update(p, 17, 1 << 2 | 1 << 5 | 1 << 7 | 1 << 8 | 1 << 9 | 1 << 10
				| 1 << 11 | 1 << 13 | 1 << 14 | 1 << 15);

	_amp = p[2];
	if (p[7] != _minFreq || p[8] != _maxFreq || p[9] != _ampThresh) {
		_minFreq = p[7];
		_maxFreq = p[8];
		_ampThresh = p[9];
		_sdif->filterPartials(_minFreq, _maxFreq, ampdb(_ampThresh));
	}
	if (_sdif) {
		double time = p[5];
		_frame = _sdif->getFrame(time, SDIFfile::Interpolated);	// do this after filterPartials
		if (_frame) {
			const int frameSet = _player->setFrame(_frame);
#ifdef DEBUG
			if (frameSet) {
				printf("reqtime=%f, frame=%p, frametime=%f, numpartials=%d\n",
							time, _frame, _frame->time(), _frame->numPartials());
				_frame->dump();
			}
#endif
		}
	}
	if (p[10] != _freqScaleFactor) {
		_player->freqScaleAll(p[10]);
		_freqScaleFactor = p[10];
	}
	if (p[11] != _freqOffset) {
		_player->freqAddAll(p[11]);
		_freqOffset = p[11];
	}

	_player->retuneTranspose(p[13]);

	float sensitivity = p[14];
	if (sensitivity < 0.0f)
		sensitivity = 0.0f;
	_player->retuneSensitivity(sensitivity);

	float strength = p[15];
	if (strength < 0.0f)
		strength = 0.0f;
	else if (strength > 1.0f)
		strength = 1.0f;
	_player->retuneStrength(strength);
}

int RESYNTH::configure()
{
	_out = new float [_bufFrames * outputChannels()];
	return _out ? 0 : -1;
}

int RESYNTH::run()
{
	// If the control rate is faster than the rate synthesizeBlock is called,
	// break up the write calls into smaller chunks, each preceded by updating
	// control parameters.
	if (_controlMultiplier > 1) {
		const int blockFrames = framesToRun() / _controlMultiplier;
		const int blockSamps = blockFrames * outputChannels();
//FIXME: framesToRun() might not be == _bufFrames on final iteration!
//printf("currentFrame: %d\n", currentFrame());
		for (int i = 0; i < _controlMultiplier; i++) {
			_doupdate();
			_player->synthesizeBlock(_out, blockFrames, outputChannels(), _amp);
			rtbaddout(_out, blockFrames);
			increment(blockFrames);
		}
	}
	// If the control rate is slower than the rate synthesizeBlock is called,
	// call _doupdate() only some of the time.
	else {
		if (--_controlCount == 0) {
			_doupdate();
			_controlCount = _audioBufsPerControl;
		}
		_player->synthesizeBlock(_out, framesToRun(), outputChannels(), _amp);
		rtbaddout(_out, framesToRun());
		increment(framesToRun());
	}

	return framesToRun();
}

Instrument *makeRESYNTH()
{
	RESYNTH *inst = new RESYNTH();
	inst->set_bus_config("RESYNTH");

	return inst;
}

void rtprofile()
{
	RT_INTRO("RESYNTH", makeRESYNTH);
}

