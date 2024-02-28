/* FBRECEIVE - instrument which provides the feedback signal to RTcmix

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = buffer index

   p2 (amp) can receive updates from a table or real-time
   control source.

   FBSEND - instrument which sends its input back to FBRECEIVE

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = buffer index

   p3 (amp) can receive updates from a table or real-time
   control source.
*/

#include <stdlib.h>
#include <string.h> // memset
#include <ugens.h>
#include "FEEDBACK.h"
#include <rt.h>
#include <rtdefs.h>

struct FeedBuffer {
	int channelCount;
	float *buffer;
};

const int kMaxFeeds = 16;

static FeedBuffer sFeeds[kMaxFeeds];

FEEDBACK::FEEDBACK()
	: _bufIndex(-1), _branch(0), _amp(0.0)
{
}

// FBRECEIVE

FBRECEIVE::FBRECEIVE()
{
}

FBRECEIVE::~FBRECEIVE()
{
}

int FBRECEIVE::init(double p[], int n_args)
{
	const float outskip = p[0];
	const float dur = p[1];
    _amp = p[2];
	_bufIndex = (int) p[3];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("FBRECEIVE", "Use mono or stereo output only.");
		
	if (_bufIndex < 0 || _bufIndex >= kMaxFeeds) {
		return die("FBRECEIVE", "Illegal buffer index");
	}

	return nSamps();
}

int FBRECEIVE::configure()
{
	int status = 0;
	try {
        if (sFeeds[_bufIndex].channelCount != outputChannels()) {
            die("FBRECEIVE", "FBSEND/FBRECEIVE pairs must use the same channel counts");
            status = -1;
        }
        else {
            _feedBuffer = sFeeds[_bufIndex].buffer;
        }
	}
	catch(...) {
		status = -1;
	}
	return status;
}

void FBRECEIVE::doupdate()
{
	double p[3];
	update(p, 3);
	_amp = p[2];
}

int FBRECEIVE::run()
{
	const int samps = framesToRun() * outputChannels();
		
	for (int i = 0; i < samps; i += outputChannels()) {

		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		float out[4];

		for (int chan = 0; chan < outputChannels(); ++chan) {

			// Grab the current feedback sample, scaled by the amplitude multiplier.

			float insig = (_feedBuffer) ? _feedBuffer[i + chan] * _amp : 0;

            out[chan] = insig;
		}

		rtaddout(out);

		increment();
	}

	return framesToRun();
}

// FBSEND

FBSEND::FBSEND()
	: _in(NULL)
{
}

FBSEND::~FBSEND()
{
    delete [] sFeeds[_bufIndex].buffer;
    sFeeds[_bufIndex].buffer = NULL;
	delete [] _in;
}

int FBSEND::init(double p[], int n_args)
{
	const float outskip = p[0];
	const float inskip = p[1];
	const float dur = p[2];
    _amp = p[3];
	_bufIndex = (int) p[4];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("FBSEND", "Use mono or stereo output only.");

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	if (_bufIndex < 0 || _bufIndex >= kMaxFeeds) {
		return die("FBSEND", "Illegal buffer index");
	}
	
	return nSamps();
}

int FBSEND::configure()
{
	int status = 0;
	try {
        const int totalSamps = RTBUFSAMPS * inputChannels();
		_in = new float [totalSamps];
        sFeeds[_bufIndex].buffer = new float [totalSamps];
        sFeeds[_bufIndex].channelCount = outputChannels();
		_feedBuffer = sFeeds[_bufIndex].buffer;
        memset(_feedBuffer, 0, totalSamps);
    }
	catch(...) {
		status = -1;
	}
	return status;	// IMPORTANT: Return 0 on success, and -1 on failure.
}

void FBSEND::doupdate()
{
	double p[4];
	update(p, 4);
	_amp = p[3];
}


int FBSEND::run()
{
	const int samps = framesToRun() * inputChannels();

	// Read the audio to be fed back from the input
	
	rtgetin(_in, this, samps);
		
	for (int i = 0; i < samps; i += inputChannels()) {

		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		float out[4];

		for (int chan = 0; chan < inputChannels(); ++chan) {

			// Grab the current input sample, scaled by the amplitude multiplier.

			float insig = _in[i + chan] * _amp;

			// Write the sample to both the feedback buffer and to the output
			
			_feedBuffer[i + chan] = insig;
			
			out[chan] = insig;
		}

		rtaddout(out);

		increment();
	}

	return framesToRun();
}

Instrument *makeFBRECEIVE()
{
	FBRECEIVE *inst = new FBRECEIVE();
	inst->set_bus_config("FBRECEIVE");

	return inst;
}

Instrument *makeFBSEND()
{
	FBSEND *inst = new FBSEND();
	inst->set_bus_config("FBSEND");

	return inst;
}

void rtprofile()
{
	RT_INTRO("FBRECEIVE", makeFBRECEIVE);
	RT_INTRO("FBSEND", makeFBSEND);
}


