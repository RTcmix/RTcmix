#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <mixerr.h>
#include <Instrument.h>
#include <notetags.h>
#include <rt.h>
#include <rtdefs.h>
#include "MIXN.h"
#include <ugens.h>

#include "funcs.h"

extern int resetval;

MIXN::MIXN() : Instrument()
{
  in = NULL;
  my_aud_locs = aud_locs;
  my_spk_locs = spk_locs;
  my_ratefs = ratefs;
  my_num_rates = num_rates;
  my_cur_rate = cur_rate;
  my_cur_point = cur_point;
  my_num_points = num_points;
  my_out_chan_amp = new float[MAXBUS];
  my_tot_dist = tot_dist;
  my_n_spk = n_spk;
  my_use_path = use_path;
  my_use_rates = use_rates;
  my_cycle = cycle;
}

MIXN::~MIXN()
{
  delete [] in;
}

int MIXN::init(double p[], int n_args)
{
  // p0 = outsk
  // p1 = insk
  // p2 = duration (-endtime)
  // p3 = inchan
  // p4-n = channel amps
  // we're (still) stashing the setline info in gen table 1

  int i;
  float dur;

  if (p[2] < 0.0) p[2] = -p[2] - p[1];

  outskip = p[0];
  inskip = p[1];
  dur = p[2];

  if (rtsetoutput(outskip, dur, this) != 0)
	  return DONT_SCHEDULE;
  if (rtsetinput(inskip, this) != 0)
	  return DONT_SCHEDULE;

  inchan = p[3];
  amp = p[4];
	
  amp_count = 0;
  outputchans = outputChannels();
  for (i=0; i < outputchans; i++) {
	my_out_chan_amp[i] = 0.0;
  }

  // Make sure this works with trajectories, etc...
  for (i = 0; i < n_args-5; i++) {
	my_out_chan_amp[i] = p[i+5];
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
	tableset(SR, dur, amplen, tabs);
  }
  else
	printf("Setting phrase curve to all 1's\n");

  skip = (int)(SR/(float)resetval); // how often to update amp curve, default 200/sec.

  //  if (inchan >= inputChannels())
  //  warn("MIXN","You are requesting channel %2.f of a %d channel file.\n",inchan+1,inputChannels());

  return(this->mytag);
}

int MIXN::configure()
{
    in = new float [RTBUFSAMPS * inputChannels()];
	return 0;
}

int MIXN::run()
{
  int i,j,k,outframes;
  float aamp;
  int branch;
  float *outp;
  float t_out_amp[8];  /* FIXME make this more flexible later */
  float t_dur;
  float sig;
  int finalsamp;

  aamp = 0;

  aud_locs = my_aud_locs;
  spk_locs = my_spk_locs;
  ratefs = my_ratefs;
  num_rates = my_num_rates;
  cur_rate = my_cur_rate;
  cur_point = my_cur_point;
  num_points = my_num_points;
  out_chan_amp = my_out_chan_amp;
  tot_dist = my_tot_dist;
  n_spk = my_n_spk;
  use_path = my_use_path;
  use_rates = my_use_rates;
  cycle = my_cycle;

  outframes = framesToRun()*inputChannels();

  rtgetin(in, this, outframes);

  outp = outbuf;  // Use private pointer to Inst::outbuf

  branch = 0;
  for (i = 0; i < outframes; i += inputChannels())  {
	if (--branch < 0) {
#ifdef RTUPDATE
	  if (tags_on) {
		for (j=0;j<8;j++) {
		  t_out_amp[j] = rtupdate(this->mytag,j+4);
		  if (t_out_amp[j] != NOPUPDATE) {
			out_chan_amp[j] = t_out_amp[j];
		  }
		}
	  }
#endif
	  if (amptable)
		aamp = tablei(cursamp, amptable, tabs) * amp;
	  else
		aamp = amp;
	  if (use_path) {
		update_amps(cursamp);
	  }
	  branch = skip;
	}

	if (inchan < inputChannels())
	  sig = in[i + (int)inchan] * aamp;
	else
	  sig = 0;


	for (j = 0; j < outputchans; j++) {
	  outp[j] = sig * out_chan_amp[j];
	}
	outp += outputchans;
	cursamp++;
  }

  my_cur_rate = cur_rate;
  my_cur_point = cur_point;

  return i;
}

Instrument*
makeMIXN()
{
  MIXN *inst;

  inst = new MIXN();
  inst->set_bus_config("MIXN");
#ifdef RTUPDATE
  inst->set_instnum("MIXN");
#endif
  return inst;
}

void
rtprofile()
{
  RT_INTRO("MIXN",makeMIXN);
}

