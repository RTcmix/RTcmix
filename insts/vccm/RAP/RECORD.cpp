#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include <globals.h>
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

int RECORD::init(float p[], short n_args)
{
// p0 = start; p1 = duration (-endtime); p2 = inchan; p3 = audio index

	int i;

	if (p[1] < 0.0) p[1] = -p[1] - p[0];

	nsamps = rtsetoutput(p[0], p[1], this);
	rtsetinput(p[0], this);

	dur = p[1];
	inchan = (int)p[2];
	aud_idx = (int)p[3];

	if (inchan > inputchans) {
			die("RECORD", "You wanted input channel %d, but have only specified "
							"%d input channels", p[2], inputchans);
	}
	if (aud_idx >= MAX_AUD_IDX) {
	  die("RECORD", "You wanted to use audio index %d, but your RTcmix version"
		  "has only been compiled for %d", aud_idx, MAX_AUD_IDX);
	}

	return(nsamps);
}

int RECORD::run()
{
	int i,j,k,rsamps;
	float sig;

	Instrument::run();

	// Allocate some RAM to store audio in
	if (temp_buff[aud_idx] == NULL)
		temp_buff[aud_idx] = new float [dur * SR];

	if (in == NULL)    /* first time, so allocate it */
	  in = new float [RTBUFSAMPS * inputchans];

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	for (i = 0; i < rsamps; i += inputchans)  {

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




