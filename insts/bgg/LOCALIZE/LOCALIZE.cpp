/* LOCALIZE - localize sound source

	p[0] = output skip time
	p[1] = input skip time
	p[2] = duration
	*p[3] = overall amp
	*p[4] = source x
	*p[5] = source y
	*p[6] = source z
	*p[7] = dest x
	*p[8] = dest y
	*p[9] = dest z
	*p[10] = headwidth (units)
	p[11] = feet/unit scaler
	p[12] = input channel
	p[13] = behind head filter on/off
	p[14] = amp/distance calculation flag
		0: no amp/distance
		1: linear amp/distance
		2: inverse square amp/distance
	p[15] = minimum amp/distance multiplier
	p[16] = maximum distance (for linear amp/distance scaling)

	* p-fields marked with an asterisk can receive dynamic updates
	from a table or real-time control source

	speed of sound set at 1000 ft/second

	NOTES:
	This instrument is designed for use in RTcmix/Unity to localize
	GameObject sound sources.  The 'source' x/y/z coordinates are generally
	the coordinates of the GameObject making the sound *relative* to the
	AudioListener receiving the sound (which means that p7/p8/p9 are
	usually set to 0.0).  These relative coordinates can be ascertained
	using the getMyTransform.cs script as a component on the AudioListener.
	This allows the instrument to take into account the 3-D rotation of the
	AudioListener as well as the location in 3-space.

	One non-Unity thing:  the x/y plane in LOCALIZE is the lateral plane,
	with the z axis going up and down.  In Unity the x/z plane is the
	lateral plane with the y axis up/down.  They need to be flipped when
	using LOCALIZE; i.e. put the z-coordinate in place of the y-coordinate
	(and vice-versa).  I'm not sure why they did this in Unity, but I
	prefer x/y to be the primary lateral plane.  So there!

	The 'head filter' amp rolloff when the source is to the left or
	right is a simple 60% decrease when the source is 90 degrees right
	or left.  This was done 'by ear'.  Also, the behind-the-head filter
	is only a simple low-pass at present.  Both of these factors increase
	as the sound source moves to the right/left or behind the listener.
	Plans are to incorporate a more robust HRTF filter in the future.

	Brad Garton, 11/2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include "LOCALIZE.h"          // declarations for this instrument class
#include <rt.h>
#include <rtdefs.h>


// constructor
LOCALIZE::LOCALIZE()
{
}


// destructor
LOCALIZE::~LOCALIZE()
{
}


// this is called when the instrument initializes and gets scheduled
int LOCALIZE::init(double p[], int n_args)
{
	float mult;
	int flipper;

	// set up input and output
	int returnval = rtsetoutput(p[0], p[2], this);
	if (returnval == -1) return DONT_SCHEDULE;

	returnval = rtsetinput(p[1], this);
	if (returnval == -1) return DONT_SCHEDULE;

	// get the p-field values we will use later
	_amp = p[3];

	// angle + distance from source to destination
	dist = sqrt( 
		pow((p[4]-p[7]), 2.0) +
		pow((p[5]-p[8]), 2.0) +
		pow((p[6]-p[9]), 2.0)
		);
	theta = asin((p[5] - p[8])/dist);

	headdelay = (p[10]*p[11])/1000.0; // simple assumption, sound 1000 ft/sec

	// calculate the ear delays
	flipper = 0;
	if (theta > 0.0) {
		if (p[4] - p[7] > 0.0) {
			// upper right
			mult = 1.0 - theta/M_PI_2;
		} else {
			// upper left
			mult = 1.0 - theta/M_PI_2;
			// flip
			flipper = 1;
		}
	} else {
		if (p[4] - p[7] > 0.0) {
			// lower right
			mult = 1.0 - (-theta/M_PI_2);
		} else {
			// lower left
			mult = 1.0 - (-theta/M_PI_2);
			// flip
			flipper = 1;
		}
	}

	// simple head shadow (decrease amp by 0.6 when full R or L)
	if (flipper == 0) {
		_delayR = 0;
		_delayL = mult * headdelay * SR;
		_ampL = 1.0 - (mult * 0.6);
	} else {
		_delayR = mult * headdelay * SR;
		_delayL = 0;
		_ampR = 1.0 - (mult * 0.6);
	}

	// set up delay lines
	theDelayR = new Odelayi(0.1 * SR);
	theDelayL = new Odelayi(0.1 * SR);

	// set up our sample-reading
	theIn = new Ortgetin(this);

	// check and calculate behind head (head filter)
	// simple lowpass filter for now
	_dofilt = 0;
	if ((int)p[13] == 1) {
		float filt_cutoff = 0.0;

		if (theta < 0.0) // behind us
			filt_cutoff = -theta/M_PI_2;

		// from full to 500 Hz cutoff totally behind, tracking exponentially (3)
		headfilt = new Oonepole(44100.0, 500.0 +
			pow((1.0 - filt_cutoff), 3.0)   * 21000.0);

		// cut the amplitude slightly
		_amp *= (1.0 - (filt_cutoff * 0.3));  // -theta/M_PI_2 already calculated

		_dofilt = 1;
	}


	// do amp/distance calculation
	_doampdist = 0;
	if ((int)p[14] == 1) { // linear
		float distcut;

		_mindistamp = p[15];
		_totlindist = p[16];
		if (dist < _totlindist)
			distcut = 1.0 - dist/_totlindist;
		else distcut = -1;
		if (distcut < _mindistamp) distcut = _mindistamp;
		_amp *= distcut;

		_doampdist = 1;
	}
	if ((int)p[14] == 2) { // inverse square
		float distcut;

		_mindistamp = p[15];
		if (dist*p[11] >= 1.0)
			distcut = 1.0/(dist * p[11]);
		else
			distcut = 1.0;
		if (distcut < _mindistamp) distcut = _mindistamp;
		_amp *= distcut;

		_doampdist = 2;
	}

	_branch = 0;
	_inputchan = p[12];

	return nSamps();
}

// this is called just before the instrument starts to run samples
int LOCALIZE::configure()
{
	return 0;	// IMPORTANT: Return 0 on success, and -1 on failure.
}


// Called at the control rate to update PFIELD parameters
void LOCALIZE::doupdate()
{
	double p[12]; // update the first 12 pfields
	float mult;
	int flipper;
	float angsin;

	update(p, 12);

	_amp = p[3];

	dist = sqrt( 
		pow((p[4]-p[7]), 2.0) +
		pow((p[5]-p[8]), 2.0) +
		pow((p[6]-p[9]), 2.0)
		);

	// gaurd against situations where the two co-exist in the same coords
	if (dist == 0.0) return;

	angsin = (p[5] - p[8])/dist;
	if (angsin > 1.0) angsin = 0.9999;
	theta = asin(angsin);

	headdelay = (p[10]*p[11])/1000.0;

	flipper = 0;
	if (theta > 0.0) {
		if (p[4] - p[7] > 0.0) {
			// upper right
			mult = 1.0 - theta/M_PI_2;
		} else {
			// upper left
			mult = 1.0 - theta/M_PI_2;
			// flip
			flipper = 1;
		}
	} else {
		if (p[4] - p[7] > 0.0) {
			// lower right
			mult = 1.0 - (-theta/M_PI_2);
		} else {
			// lower left
			mult = 1.0 - (-theta/M_PI_2);
			// flip
			flipper = 1;
		}
	}

	if (flipper == 0) {
		_delayR = 0;
		_delayL = mult * headdelay * SR;
		_ampL = 1.0 - (mult * 0.6);
	} else {
		_delayR = mult * headdelay * SR;
		_delayL = 0;
		_ampR = 1.0 - (mult * 0.6);
	}

	if (_dofilt == 1) {
		float filt_cutoff = 0.0;

      if (theta < 0.0) {
         filt_cutoff = -theta/M_PI_2;
	      headfilt->setfreq(500.0 + 
				pow((1.0 - filt_cutoff), 3.0)   * 21000.0);

			_amp *= (1.0 - (filt_cutoff * 0.3));  // -theta/M_PI_2 already calculated
		} else {
			headfilt->setfreq(22050.0);
		}
	}

	
	if (_doampdist == 1) { // linear
		float distcut;

		if (dist < _totlindist)
			distcut = 1.0 - dist/_totlindist;
		else distcut = -1;
		if (distcut < _mindistamp) distcut = _mindistamp;
		_amp *= distcut;
	}
	if (_doampdist == 2) { // inverse square
		float distcut;

		if (dist*p[11] >= 1.0)
			distcut = 1.0/(dist * p[11]);
		else
			distcut = 1.0;
		if (distcut < _mindistamp) distcut = _mindistamp;
		_amp *= distcut;
	}
}


// Called by the scheduler for every time slice in which this instrument
// should run.  This is where the real work of the instrument is done.
int LOCALIZE::run()
{
	float out[2];
	float in[2]; // mono will only fill in[_inputchan]

	for (int i = 0; i < framesToRun(); i++) {
		if (--_branch <= 0) {
				doupdate();
				_branch = getSkip();
		}

		theIn->next(in);

		theDelayR->putsamp(in[_inputchan]);
		theDelayL->putsamp(in[_inputchan]);

		if (_delayL < 2.0) 
			out[0] = in[_inputchan] * _amp;
		else 
			out[0] = theDelayL->getsamp(_delayL) * _amp * _ampL;
		if (_delayR < 2.0) 
			out[1] = in[_inputchan] * _amp;
		else
			out[1] = theDelayR->getsamp(_delayR) * _amp * _ampR;

		if (_dofilt == 1) {
			out[0] = headfilt->next(out[0]);
			out[1] = headfilt->next(out[1]);
		}

		rtaddout(out);

		increment();
	}


	return framesToRun(); // how many we processed
}


// The scheduler calls this to create an instance of this instrument
// and to set up the bus-routing fields in the base Instrument class.
// This happens for every "note" in a score.
Instrument *makeLOCALIZE()
{
	LOCALIZE *inst = new LOCALIZE();
	inst->set_bus_config("LOCALIZE");

	return inst;
}


// The rtprofile introduces this instrument to the RTcmix core, and
// associates a script function name (in quotes below) with the instrument.
// This is the name the instrument goes by in a script.

// the EMBEDDED stuff is because rtprofile is only called once with the
// instantiation of the RTcmix process in the calling environment

#ifndef EMBEDDED
void rtprofile()
{
	RT_INTRO("LOCALIZE", makeLOCALIZE);
}
#endif

