#include <iostream.h>
#include <unistd.h>
#include <stdio.h>
#include <mixerr.h>
#include <Instrument.h>
#include <notetags.h>
#include <rt.h>
#include <rtdefs.h>
#include <globals.h>
#include "MIXN.h"

extern "C" {
#include <ugens.h>
  extern int resetval;
  extern float out_chan_amp[MAXBUS];
  extern void update_amps(long);
  extern Bool use_path;
  extern int max_speak;
}


MIXN::MIXN() : Instrument()
{
  in = NULL;
}

MIXN::~MIXN()
{
  delete [] in;
}

int MIXN::init(float p[], short n_args)
{
  // p0 = outsk
  // p1 = insk
  // p2 = duration (-endtime)
  // p3 = inchan
  // p4-n = channel amps
  // we're (still) stashing the setline info in gen table 1

  int i;

  if (p[2] < 0.0) p[2] = -p[2] - p[1];

  outskip = p[0];
  inskip = p[1];
  dur = p[2];

  nsamps = rtsetoutput(outskip, dur, this);
  rtsetinput(inskip, this);

  inchan = p[3];
  amp = p[4];
	
  amp_count = 0;
  for (i=0; i < outputchans; i++) {
	out_chan_amp[i] = 0.0;
  }


  // Make sure this works with trajectories, etc...
  for (i = 0; i < n_args-5; i++) {
	out_chan_amp[i] = p[i+5];
	if (i > outputchans) {
	  fprintf(stderr, "You wanted output channel %d, but have only "
			  "specified %d output channels\n",
			  i, outputchans);
	  exit(-1);
	}
  }
  amp_count = i;  // Watch out for this one

  amptable = floc(1);
  if (amptable) {
	int amplen = fsize(1);
	tableset(dur, amplen, tabs);
  }
  else
	printf("Setting phrase curve to all 1's\n");

  skip = (int)(SR/(float)resetval); // how often to update amp curve, default 200/sec.

  return(nsamps);
}

int MIXN::run()
{
  int i,j,k,outframes;
  float aamp;
  int branch;
  float *outp;
  float t_out_amp[8];  /* FIXME make this more flexible later */
  float t_dur;
  int finalsamp;

  if (in == NULL)    /* first time, so allocate it */
    in = new float [RTBUFSAMPS * inputchans];

  Instrument::run();

  outframes = chunksamps*inputchans;

  rtgetin(in, this, outframes);

  outp = outbuf;  // Use private pointer to Inst::outbuf

  branch = 0;
  for (i = 0; i < outframes; i += inputchans)  {
	if (--branch < 0) {
	  if (tags_on) {
		for (j=0;j<8;j++) {
		  t_out_amp[j] = rtupdate(this->mytag,j+4);
		  if (t_out_amp[j] != NOPUPDATE) {
			out_chan_amp[j] = t_out_amp[j];
		  }
		}
	  }
	  if (amptable)
		aamp = tablei(cursamp, amptable, tabs) * amp;
	  else
		aamp = amp;
	  if (use_path) {
		update_amps(cursamp);
	  }
	  branch = skip;
	}

	float sig = in[i + (int)inchan] * aamp;

	for (j = 0; j < outputchans; j++) {
	  outp[j] = sig * out_chan_amp[j];
	}
	outp += outputchans;
	cursamp++;
  }
  return i;
}



Instrument*
makeMIXN()
{
  MIXN *inst;

  inst = new MIXN();
  inst->set_bus_config("MIXN");

  return inst;
}

void
rtprofile()
{
  RT_INTRO("MIXN",makeMIXN);
}

