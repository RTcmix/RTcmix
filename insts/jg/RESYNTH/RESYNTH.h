// Copyright (C) 2012 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include <Instrument.h>

class Frame;
class SDIFfile;
class PartialFramePlayer;

class RESYNTH : public Instrument {
public:
	RESYNTH();
	virtual ~RESYNTH();
	int usage() const;
	virtual int init(double p[], int n_args);
	virtual int configure();
	virtual int run();

private:
	void _doupdate();

	int _nargs, _bufFrames, _audioBufsPerControl, _controlCount, _controlMultiplier;
	float _amp, _minFreq, _maxFreq, _ampThresh;
	float _freqScaleFactor, _freqOffset;
	float *_out;
	Frame *_frame;
	SDIFfile *_sdif;
	PartialFramePlayer *_player;
};

