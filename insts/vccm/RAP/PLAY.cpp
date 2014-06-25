#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
#include "PLAY.h"

extern float *temp_buff[];

PLAY::PLAY() : Instrument()
{
	in = NULL;
}

PLAY::~PLAY()
{
	delete [] in;
}

int PLAY::init(double p[], int n_args)
{
// p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; p4 = aud_idx
// setline info is in gen table 1

	int i;

	if (p[2] < 0.0) p[2] = -p[2] - p[1];

	if (rtsetoutput(p[0], p[2], this) != 0)
		return DONT_SCHEDULE;
	/* rtsetinput(p[1], this); */

	dur = p[2];
	amp = p[3];
	aud_idx = (int)p[4];
	idx_samp = (int)(p[1]*SR);

	if (outputChannels() > 2) {
		return die("PLAY", "This is a MONO output instrument.  Reset your bus_config to use only 1 output bus\n");
	}

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, p[2], amplen, tabs);
	}
	else
		rtcmix_advise("PLAY", "Setting phrase curve to all 1's.");

	skip = (int)(SR/(float)resetval);

	return(this->mytag);
}

int PLAY::configure()
{
	in = new float [RTBUFSAMPS];
	return 0;
}

int PLAY::run()
{
	int i,j,k,rsamps;
	float out[MAXBUS];
	float aamp=0.0f;
	int branch;

	rsamps = framesToRun();

	/* Read input from temp_buffer */
	for(i=0;i<rsamps;i++) {
	  in[i] = temp_buff[aud_idx][idx_samp++];
	}

	branch = 0;
	for (i = 0; i < rsamps; i++)  {
	  if (--branch < 0) {
		if (amptable)
		  aamp = tablei(cursamp, amptable, tabs) * amp;
		else
		  aamp = amp;
		branch = skip;
	  }
	  
	  /* This is all mono ... so we just put 1 sample in the 1st slot */
	  for (j = 1; j < outputchans; j++) {
		out[j] = 0.0;
	  }
	  out[0] = in[i] * aamp;
	  
	  rtaddout(out);
	  cursamp++;
	}
	return i;
}

Instrument*
makePLAY()
{
	PLAY *inst;
	inst = new PLAY();
	inst->set_bus_config("PLAY");
#ifdef RTUPDATE
	inst->set_instnum("PLAY");
#endif
	return inst;
}


