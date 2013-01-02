#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include "COMBFILT.h"
#include <rt.h>
#include <rtdefs.h>
#include <combs.h>
#include <math.h>

COMBFILT::COMBFILT() : Instrument()
{
  in = NULL;
  x = NULL;
  y = NULL;
}

COMBFILT::~COMBFILT()
{
  delete [] in;
  delete [] x;
  delete [] y;
}


int COMBFILT::init(double p[], int n_args)
{

  // p0 = outsk; p1 = insk; p2 = input dur; p3 = amp mult
  // p4 = pitch; p5 = a (input mult); p6 = b (delay mult); 
  // p7 = filter type (FIR/IIR)
  // p8 = wetdry;  p9 = inchan [optional]; p10 = spread [optional]
  // p11 = rise
  // p12 = sustain
  // p13 = decay
  // assumes function table 1 is the amplitude envelope

  if (rtsetinput(p[1], this) != 0)
	  return DONT_SCHEDULE;
  if (rtsetoutput(p[0], p[2], this) != 0)
	  return DONT_SCHEDULE;

  insamps = (int)(p[2] * SR);

  if (p[4] < 15.0)
	combfreq = cpspch(p[4]);
  else
	combfreq = p[4];
	
  // FIXME:  need check here:  combfreq < SR/2
  delay = (int)rint(SR/combfreq);

  amptable = floc(1);
  if (amptable) {
	int amplen = fsize(1);
	tableset(SR, p[2], amplen, tabs);
  }
  else
	rtcmix_advise("COMBFILT", "Setting phrase curve to all 1's.");

  amp = p[3];
  a = p[5];
  b = p[6];
  // FIXME:  need check here:  type < 2
  type = (int)p[7];
  skip = (int)(SR/(float)resetval); // how often to update amp curve
  inchan = (int)p[9];
  if ((inchan+1) > inputChannels())
	return die("COMBFILT", "You asked for channel %d of a %d-channel file.", 
				inchan,inputChannels());
  wetdry = p[8];
  spread = p[10];

  maxdelay = (int)rint(SR);
  runsamp = 0;

  return(this->mytag);
}

int COMBFILT::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	x = new float[maxdelay];
	y = new float[maxdelay];
	for (int i=0;i<maxdelay;i++) {
	  x[i] = y[i] = 0.0;
	}
	return 0;
}

int COMBFILT::run()
{
  int i,j,rsamps;
  float out[2];
  float aamp;
  int branch;
  int tsize;
  int tdelay;
  float tdur, tamp, tpitch, ta, tb, ttype, twetdry, tchan, tspread;
  int finalsamp;

  tsize = (RTBUFSAMPS * inputChannels());

  rsamps = framesToRun()*inputChannels();

  rtgetin(in, this, rsamps);

  aamp = amp;        /* in case amptable == NULL */

  branch = 0;
  for (i = 0; i < rsamps; i += inputChannels())  {
	if (cursamp > insamps) {
	  for (j = 0; j < inputChannels(); j++) 
		in[i+j] = 0.0;
	}

	if (--branch < 0) {
	  // End P-Field Updating - - - - - - - - - - - - - - - - - - - - -
	  if (amptable)
		aamp = table(cursamp, amptable, tabs) * amp;
	  branch = skip;
	}

	if (runsamp >= maxdelay) {
	  runsamp = 0;
	}

	x[runsamp] = in[i];

	delaysamp = (int)runsamp-delay;
	if (delaysamp < 0) {
	  delaysamp = maxdelay + delaysamp;
	}

	/* FIR comb filter */
	if (type == FIR)
	  out[0] = ((a * in[i]) + (b * x[delaysamp])) * aamp;
	/* IIR comb filter */
	else if (type == IIR)
	  out[0] = ((a * in[i]) + (b * y[delaysamp])) * aamp;

	y[runsamp] = out[0];

	runsamp++;

	if (outputchans == 2) {
	  out[1] = out[0] * (1.0 - spread);
	  out[0] *= spread;
	}

	rtaddout(out);
	cursamp++;
  }
  return(i);
}



Instrument*
makeCOMBFILT()
{
  COMBFILT *inst;

  inst = new COMBFILT();
  inst->set_bus_config("COMBFILT");
  return inst;
}

void
rtprofile()
{
  RT_INTRO("COMBFILT",makeCOMBFILT);
}

