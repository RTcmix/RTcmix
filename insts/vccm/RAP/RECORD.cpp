#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
#include "RECORD.h"

float *temp_buff[MAX_AUD_IDX];
static Bool first_run = YES;

RECORD::RECORD() : Instrument()
{
  int i;
  if (first_run) {
	for (i=0;i<MAX_AUD_IDX;i++) 
	  temp_buff[i] = NULL;
	first_run = NO;
  }
  in = NULL;
}

RECORD::~RECORD()
{
	delete [] in;
}

int RECORD::init(double p[], int n_args)
{
// p0 = start; p1 = duration (-endtime); p2 = inchan; p3 = audio index

	int i;

	if (p[1] < 0.0) p[1] = -p[1] - p[0];

	if (rtsetoutput(p[0], p[1], this) != 0)
		return DONT_SCHEDULE;
	rtsetinput(p[0], this);

	dur = p[1];
	inchan = (int)p[2];
	aud_idx = (int)p[3];

	if (inchan > inputChannels()) {
		return die("RECORD", 
					"You wanted input channel %d, but have only specified "
						"%d input channels", p[2], inputChannels());
	}
	if (aud_idx >= MAX_AUD_IDX) {
		return die("RECORD", 
				"You wanted to use audio index %d, but your RTcmix version"
			  "has only been compiled for %d", aud_idx, MAX_AUD_IDX);
	}

	return(this->mytag);
}

int RECORD::configure()
{
	// Allocate some RAM to store audio in
	int idur = (int) ceil(dur);
	if (temp_buff[aud_idx] == NULL)
		temp_buff[aud_idx] = new float [idur * (int)SR];
    in = new float [RTBUFSAMPS * inputChannels()];
	return 0;
}

int RECORD::run()
{
	int i,j,k,rsamps;
	float sig;

	rsamps = framesToRun()*inputChannels();

	rtgetin(in, this, rsamps);

	for (i = 0; i < rsamps; i += inputChannels())  {

	  sig = in[i + (int)inchan];
	  temp_buff[aud_idx][cursamp] = sig;

	  cursamp++;

	}
	return i;
}

Instrument*
makeRECORD()
{
	RECORD *inst;

	inst = new RECORD();
	inst->set_bus_config("RECORD");
	return inst;
}




