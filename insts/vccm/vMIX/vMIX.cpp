#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <rt.h>
#include <rtdefs.h>
#include "vMIX.h"

// extern Bool stop_hold;
extern Bool start_fade;
// extern Bool fade_started;

vMIX::vMIX() : Instrument()
{
	in = NULL;
	branch = 0;
}

vMIX::~vMIX()
{
	delete [] in;
}

int vMIX::init(double p[], int n_args)
{
	// p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; 
	// p4 = fade time
	// p5-n = channel mix matrix
	// we're stashing the setline info in gen table 1

	int i;
	float fade_time;

	if (p[2] < 0.0) p[2] = -p[2] - p[1];

	if (rtsetoutput(p[0], p[2], this) != 0)
		return DONT_SCHEDULE;
	if (rtsetinput(p[1], this) != 0)
		return DONT_SCHEDULE;

	amp = p[3];

	fade_time = p[4];
	fade_samps = (int)(fade_time*SR);
	fade_samp = 0;

	for (i = 0; i < inputChannels(); i++) {
		outchan[i] = (int)p[i+5];
		if (outchan[i] + 1 > outputchans) {
			die("vMIX", "You wanted output channel %d, but have only specified "
			    "%d output channels", outchan[i], outputchans);
		}
	}

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, p[2], amplen, tabs);
	}
	else
		rtcmix_advise("vMIX", "Setting phrase curve to all 1's.");

	if (fade_time) {
	  	fade_table = floc(2);
		if (fade_table) {
		   	int famplen = fsize(2);
			tableset(SR, fade_time, famplen, f_tabs);
		}
	}	

	aamp = amp;
	skip = (int)(SR/(float)resetval);

	return(this->mytag);
}

int vMIX::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	start_fade = NO;
	fade_started = NO;
	return 0;
}

int vMIX::run()
{
	int i,j,k,rsamps, finalsamp;
	float out[MAXBUS];
	float tamp;

	rsamps = framesToRun()*inputChannels();

	rtgetin(in, this, rsamps);

	for (i = 0; i < rsamps; i += inputChannels())  {
		if (--branch < 0) {
			if ((start_fade) || (fade_started)) {
				if (!fade_started) {
					finalsamp = i_chunkstart+i+fade_samps;
					this->setendsamp(finalsamp);
				}
				fade_started = YES;
				if (fade_table)
					amp = tablei(fade_samp, fade_table, f_tabs) * aamp;
				else
					amp = aamp;
			}

			if (amptable)
				aamp = tablei(cursamp, amptable, tabs) * amp;
			else
				aamp = amp;

			branch = skip;
		}

		for (j = 0; j < outputchans; j++) {
			out[j] = 0.0;
			for (k = 0; k < inputChannels(); k++) {
				if (outchan[k] == j)
					out[j] += in[i+k] * aamp;
			}
		}

		rtaddout(out);
		if (fade_started)
			fade_samp++;
		cursamp++;
	}
	return i;
}



Instrument*
makevMIX()
{
	vMIX *inst;

	inst = new vMIX();
	inst->set_bus_config("vMIX");
	return inst;
}

void
rtprofile()
{
	RT_INTRO("vMIX",makevMIX);
	RT_INTRO("FADE_vMIX",makeFADE_vMIX);
}


