#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "COMBFILT.h"
#include <rt.h>
#include <rtdefs.h>
#include <combs.h>
//#include <globals.h>
#include <math.h>
#include <rtupdate.h>

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


int COMBFILT::init(float p[], int n_args)
{

  // p0 = outsk; p1 = insk; p2 = input dur; p3 = amp mult
  // p4 = pitch; p5 = a (input mult); p6 = b (delay mult); 
  // p7 = filter type (FIR/IIR)
  // p8 = wetdry;  p9 = inchan [optional]; p10 = spread [optional]
  // p11 = rise
  // p12 = sustain
  // p13 = decay
  // assumes function table 1 is the amplitude envelope
  Instrument::init(p, n_args);
  rtsetinput(p[1], this);
  nsamps = rtsetoutput(p[0], p[2], this);
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
	tableset(p[2], amplen, tabs);
  }
  else
	advise("COMBFILT", "Setting phrase curve to all 1's.");

  amp = p[3];
  a = p[5];
  b = p[6];
  // FIXME:  need check here:  type < 2
  type = (int)p[7];
  skip = (int)(SR/(float)resetval); // how often to update amp curve
  inchan = (int)p[9];
  if ((inchan+1) > inputchans)
	die("COMBFILT", "You asked for channel %d of a %d-channel file.", 
		inchan,inputchans);
  wetdry = p[8];
  spread = p[10];

  maxdelay = (int)rint(SR);
  runsamp = 0;

  return(this->mytag);
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

  tsize = (RTBUFSAMPS * inputchans);

  if (in == NULL) {    /* first time, allocate buffers */
	in = new float [RTBUFSAMPS * inputchans];
	x = new float[maxdelay];
	y = new float[maxdelay];
	for (i=0;i<maxdelay;i++) {
	  x[i] = y[i] = 0.0;
	}
  }

  Instrument::run();

  rsamps = chunksamps*inputchans;

  rtgetin(in, this, rsamps);

  aamp = amp;        /* in case amptable == NULL */

  branch = 0;
  for (i = 0; i < rsamps; i += inputchans)  {
	if (cursamp > insamps) {
	  for (j = 0; j < inputchans; j++) 
		in[i+j] = 0.0;
	}

	if (--branch < 0) {
	  // Update P-Fields + + + + + + + + + + + + + + + + + + + 
	  if(tags_on)
	  {
		tdur = rtupdate(this->mytag, 2);
		if((tdur != NOPUPDATE) /* && rsd_env != DECAY */)
		{
	//	  finalsamp = i_chunkstart + i + decay_samps;
		  this->setendsamp(finalsamp);
	//	  if(decay_table)
	//		rsd_env = DECAY;
	//	  rsd_samp = 0;
		  amp = aamp;
		}
	    tamp = rtupdate(this->mytag, 3);
		if(tamp != NOPUPDATE)
		{
		  amp = tamp;
		  aamp = amp;
		}
		tpitch = rtupdate(this->mytag, 4);
		if(tpitch != NOPUPDATE)
		{
		  if(tpitch < 15.0)
			tpitch = cpspch(tpitch);
		  combfreq = tpitch;
		  delay = (int)rint(SR/combfreq);
		}
		ta = rtupdate(this->mytag, 5);
		if(ta != NOPUPDATE)
		  a = ta;
		tb = rtupdate(this->mytag, 6);
		if(tb != NOPUPDATE)
		  b = tb;
		ttype = rtupdate(this->mytag, 7);
		if(ttype != NOPUPDATE)
		  type = (int)ttype;
		twetdry = rtupdate(this->mytag, 8);
		if(twetdry != NOPUPDATE)
		  wetdry = twetdry;
		tchan = rtupdate(this->mytag, 9);
		if(tchan != NOPUPDATE)
		{
		  inchan = (int)tchan;
		  if ((inchan+1) > inputchans)
			die("COMBFILT", "You asked for channel %d of a %d-channel file.", 
		        inchan,inputchans);
		}   
		tspread = rtupdate(this->mytag, 10);
		if(tspread != NOPUPDATE)
		  spread = tspread;
	  }  
	  // End P-Field Updating - - - - - - - - - - - - - - - - - - - - -
	  if (amptable)
		aamp = table(cursamp, amptable, tabs) * amp;
	  branch = skip;
	}

	x[runsamp] = in[i];

	delaysamp = (int)runsamp-delay;
	if (delaysamp < 1) {
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
	if (runsamp > maxdelay) {
	  runsamp = 0;
	}

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

  inst->set_instnum("COMBFILT");
  return inst;
}

void
rtprofile()
{
  RT_INTRO("COMBFILT",makeCOMBFILT);
}

