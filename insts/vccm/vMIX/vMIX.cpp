#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
#include <globals.h>
#include "vMIX.h"

extern Bool stop_hold;
extern Bool start_fade;
extern Bool fade_started;

vMIX::vMIX() : Instrument()
{
	in = NULL;
	branch = 0;
}

vMIX::~vMIX()
{
	delete [] in;
}

int vMIX::init(float p[], int n_args)
{
	// p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; 
	// p4 = fade time
	// p5-n = channel mix matrix
	// we're stashing the setline info in gen table 1

	int i;
	float fade_time;

	Instrument::init(p, n_args);

	if (p[2] < 0.0) p[2] = -p[2] - p[1];

	nsamps = rtsetoutput(p[0], p[2], this);
	rtsetinput(p[1], this);

	amp = p[3];

	fade_time = p[4];
	fade_samps = (int)(fade_time*SR);
	fade_samp = 0;

	for (i = 0; i < inputchans; i++) {
		outchan[i] = (int)p[i+5];
		if (outchan[i] + 1 > outputchans) {
			die("vMIX", "You wanted output channel %d, but have only specified "
			    "%d output channels", outchan[i], outputchans);
		}
	}

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2], amplen, tabs);
	}
	else
		advise("vMIX", "Setting phrase curve to all 1's.");

	if (fade_time) {
	  	fade_table = floc(2);
		if (fade_table) {
		   	int famplen = fsize(2);
			tableset(fade_time, famplen, f_tabs);
		}
	}	

	aamp = amp;
	skip = (int)(SR/(float)resetval);

	return(this->mytag);
}

int vMIX::run()
{
	int i,j,k,rsamps, finalsamp;
	float out[MAXBUS];
	float tamp;




	if (in == NULL) {                /* first time, so allocate it */
		in = new float [RTBUFSAMPS * inputchans];
		start_fade = NO;
		fade_started = NO;
	}

	Instrument::run();

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	for (i = 0; i < rsamps; i += inputchans)  {
		if (--branch < 0) {
			if(tags_on) {
				tamp = rtupdate(this->mytag, 3);
				if(tamp != NOPUPDATE)
					amp = tamp;
			}

			if (start_fade) {
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
			for (k = 0; k < inputchans; k++) {
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
	inst->set_instnum("vMIX");
	return inst;
}

void
rtprofile()
{
	RT_INTRO("vMIX",makevMIX);
	RT_INTRO("FADE_vMIX",makeFADE_vMIX);
}


