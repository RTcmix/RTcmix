#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
#include "STORE.h"

float *temp_buff[MAX_AUD_IDX];
static Bool first_run = YES;
int samp_marker;
int hold_samps;
int hold_start;
float hold_dur;

extern Bool stop_hold;
extern Bool fade_started;
extern Bool start_fade;

STORE::STORE() : Instrument()
{
	int i;
	if (first_run) {
		for (i=0;i<MAX_AUD_IDX;i++) 
			temp_buff[i] = NULL;
		first_run = NO;
	}
	in = NULL;
}

STORE::~STORE()
{
	delete [] in;
}

int STORE::init(double p[], int n_args)
{
	// p0 = start; p1 = duration (-endtime); p2 = hold dur; p3 = inchan; p4 = audio index

	int i;

	if (p[1] < 0.0) p[1] = -p[1] - p[0];

	if (rtsetoutput(p[0], p[1], this) != 0)
		return DONT_SCHEDULE;
	if (rtsetinput(p[0], this) != 0)
		return DONT_SCHEDULE;

	dur = p[1];
	hold_dur = p[2];
	inchan = (int)p[3];
	aud_idx = (int)p[4];
	hold_samps = (int)(hold_dur*SR);
	hold_start = (int)(p[0]*SR);
	stop_hold = NO;
	t_samp = 0;

	if (inchan > inputChannels()) {
		return die("STORE",
				   "You wanted input channel %d, but have only specified "
					"%d input channels", p[3], inputChannels());
	}
	if (aud_idx >= MAX_AUD_IDX) {
		return die("STORE",
					"You wanted to use audio index %d, but your RTcmix version"
					"has only been compiled for %d", aud_idx, MAX_AUD_IDX);
	}

	skip = (int)(SR/(float)resetval);

	return(nSamps());
}

int STORE::configure()
{
	// Allocate some RAM to store audio in
	if (temp_buff[aud_idx] == NULL)
		temp_buff[aud_idx] = new float [hold_samps];
	in = new float [RTBUFSAMPS * inputChannels()];
	return 0;
}

int STORE::run()
{
	int i,j,k,rsamps, finalsamp;
	float sig;

	rsamps = framesToRun()*inputChannels();

	rtgetin(in, this, rsamps);
	
	for (i = 0; i < rsamps; i += inputChannels())  {

		if (--branch < 0) {
			if (start_fade) {
				if (!fade_started) {
					finalsamp = i_chunkstart+i+30;
					this->setendsamp(finalsamp);
					fade_started = YES;
				}
			}
			branch = skip;
		}

		sig = in[i + (int)inchan];
		if (!stop_hold) {
			temp_buff[aud_idx][t_samp] = sig;
			t_samp++;
		}
		else {
			t_samp = 0;
			i = rsamps;
		}
		
		cursamp++;
		
		if (t_samp > hold_samps) {
			t_samp = 0;
		}

	}
	samp_marker = t_samp;

	return i;
}

Instrument*
makeSTORE()
{
	STORE *inst;

	inst = new STORE();
	inst->set_bus_config("STORE");

	return inst;
}




