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
}

// Formerly in funcs.c -----------------------------------------------------

double calc_dist(pt p1, pt p2) {
  double ret_dist;
  double x1,x2,y1,y2,a,b;

  x1 = p1.x;
  x2 = p2.x;
  y1 = p1.y;
  y2 = p2.y;
  
  a = (x2-x1);
  b = (y2-y1);

  ret_dist = sqrt(pow(a,2) + pow(b,2));
 
  return ret_dist;
}

double calc_tot_dist () {
  int i;
  double ret_dist = 0;
  pt tmp_pt;

  tmp_pt.x = 0;
  tmp_pt.y = 0;
  
  for(i=0;i<n_spk;i++)
	ret_dist += calc_dist(tmp_pt,spk_locs[i]);

  return ret_dist;
}

double speakerloc(float p[], int n_args, double pp[]) {
  int i,j;

  i=j=0;
  
  while(j<n_args) {
	spk_locs[i].x = p[j++];
	spk_locs[i].y = p[j++];
#ifdef DBUG
     printf("spk_locs[%d].x = %2.2f\n",i,spk_locs[i].x);
     printf("spk_locs[%d].y = %2.2f\n",i,spk_locs[i].y);
#endif     
     i++;
  }
  n_spk = i;
  if (i > MAXBUS) {
	fprintf(stderr,"WARNING:  set_spk_locs too many speaker locations\n");
	fprintf(stderr,"%d attempted, %d allocated\n",i, MAXBUS);
	exit(1);
  }
  tot_dist = calc_tot_dist();
}

double speakerloc_p(float p[], int n_args, double pp[]) {
  int i,j;
  double x,y,r,a;

  i=j=0;
  
  while(j<n_args) {
	r = p[j++];
	a = p[j++];

	a = 2*PI*(a/360);  /* convert to radians */

	x = r*(cos(a));
	y = r*(sin(a));

	spk_locs[i].x = x;
	spk_locs[i].y = y;
	i++;
  }
  n_spk = i;
  if (i > MAXBUS) {
	fprintf(stderr,"WARNING:  set_spk_locs too many speaker locations\n");
	fprintf(stderr,"%d attempted, %d allocated\n",i, MAXBUS);
	exit(1);
  }
  tot_dist = calc_tot_dist();
}

double rates(float p[], int n_args, double pp[]) {
  int i,j;
  double s,time;

  if (p[0] != 0) {
	fprintf(stderr,"WARNING:  rates start not = 0!\n");
  }
  if (n_args%2 != 0) {
	fprintf(stderr,"WARNING:  rates wrong number of arguments\n");
  }
  
  i=j=0;
  while(i<n_args) {
	time = p[i++];
	s = p[i++];
	ratefs[j].factor = s;
	ratefs[j].time = time;
	j++;
	max_rates++;
  }
  use_rates = YES;
}

double path(float p[], int n_args, double pp[]) {
  int i,j;
  double x,y,r,a,time,x1,x2,y1,y2,xvel,yvel,t1,t2,startsamp;

  if (p[0] != 0) {
	fprintf(stderr,"WARNING:  path start not = 0!\n");
  }
  if (n_args%3 != 0) {
	fprintf(stderr,"WARNING:  path wrong number of arguments\n");
  }
  
  i=j=0;
  while(i<n_args) {
	time = p[i++];
	x = p[i++];
	y = p[i++];

	aud_locs[j].point.x = x;
	aud_locs[j].point.y = y;
	aud_locs[j].time = time;
	aud_locs[j].xvel = 0;
	aud_locs[j].yvel = 0;
	aud_locs[j].startsamp = 0;
#ifdef DBUG
     printf("Point [%d] set:  %2.2f,%2.2f,%2.2f\n",j,time,x,y);
#endif
     j++;
	max_points++;
  }
  i=0;
  startsamp = 0;
  while(i<max_points-1) {
	x1 = aud_locs[i].point.x;
	y1 = aud_locs[i].point.y;
	t1 = aud_locs[i].time;
	x2 = aud_locs[i+1].point.x;
	y2 = aud_locs[i+1].point.y;
	t2 = aud_locs[i+1].time;
	xvel = (x2-x1)/(t2-t1);
	yvel = (y2-y1)/(t2-t1);
	startsamp += (t2-t1)*SR;
	aud_locs[i+1].xvel = xvel;
	aud_locs[i+1].yvel = yvel;
	aud_locs[i+1].startsamp = startsamp;
#ifdef DBUG
     printf("Point vel [%d] set:  %2.2f,%2.2f,%2.2f\n",i,xvel,yvel,startsamp);
#endif
     i++;
  }

  cycle = time;
  use_path = YES;
}

/* change to support xvel,yvel */

double path_p(float p[], int n_args, double pp[]) {
  int i,j;
  double x,y,r,a,time,x1,x2,y1,y2,xvel,yvel,t1,t2,startsamp;

  if (p[0] != 0) {
	fprintf(stderr,"WARNING:  path_p start not = 0!\n");
  }
  if (n_args%3 != 0) {
	fprintf(stderr,"WARNING:  path_p wrong number of arguments\n");
  }
  
  i=j=0;
  while(i<n_args) {
	time = p[i++];
	r = p[i++];
	a = p[i++];

	a = 2*PI*(a/360);  /* convert to radians */

	x = r*(cos(a));
	y = r*(sin(a));


	aud_locs[j].point.x = x;
	aud_locs[j].point.y = y;
	aud_locs[j].time = time;
	j++;
	max_points++;
  }
  i=0;
  startsamp = 0;
  while(i<max_points-1) {
	x1 = aud_locs[i].point.x;
	y1 = aud_locs[i].point.y;
	t1 = aud_locs[i].time;
	x2 = aud_locs[i+1].point.x;
	y2 = aud_locs[i+1].point.y;
	t2 = aud_locs[i+1].time;
	xvel = (x2-x1)/(t2-t1);
	yvel = (y2-y1)/(t2-t1);
	startsamp += (t2-t1)*SR;
	aud_locs[i+1].xvel = xvel;
	aud_locs[i+1].yvel = yvel;
	aud_locs[i+1].startsamp = startsamp;
#ifdef DBUG
     printf("Point [%d] set:  %2.2f,%2.2f,%2.2f\n",i,xvel,yvel,startsamp);
#endif
     i++;
  }
  use_path = YES;
}

double calc_rate(long samps) {
  double r1,r2,t1,t2,tdiff,rdiff,start_samp,end_samp,run_samps,rate;

  r1 = ratefs[cur_rate].factor;
  t1 = ratefs[cur_rate].time;

  if ((cur_rate+1) < max_rates) {
	r2 = ratefs[cur_rate+1].factor;
	t2 = ratefs[cur_rate+1].time;

	start_samp = t1*(double)SR;
	end_samp = t2*(double)SR;
	run_samps = (end_samp - start_samp);

	tdiff = (samps-start_samp)/run_samps;
	rdiff = (r2-r1);
	rdiff *= tdiff;

	if (samps > end_samp)
	  cur_rate++;

	rate = r1+rdiff;
  }
  else {
	rate = r1;
  }
  return rate;
}

void calc_loc(long samps, pt *in_point) {
  int i;
  double slope,x1,x2,y1,y2,t1,t2,tdiff,ychange,xchange,start_samp,end_samp,run_samps,xdiff,ydiff,newx,newy,mag1,mag2;
  double rate=1,rate2=1;
  double xvel,yvel,dur;
  int old_point;

  old_point = cur_point;

  x1 = aud_locs[cur_point].point.x;
  y1 = aud_locs[cur_point].point.y;
  t1 = aud_locs[cur_point].time;
  start_samp = aud_locs[cur_point].startsamp;
  
  if ((cur_point+1) < max_points) {
	x2 = aud_locs[cur_point+1].point.x;
	y2 = aud_locs[cur_point+1].point.y;
	t2 = aud_locs[cur_point+1].time;
	xvel = aud_locs[cur_point+1].xvel;
	yvel = aud_locs[cur_point+1].yvel;

#ifdef DBUG
     printf("v = %2.2f,%2.2f\n",xvel,yvel);
#endif

	/* tdiff = (samps-start_samp)/run_samps; */
	
	/* xchange = ((x2-x1)*tdiff); */
	/* ychange = ((y2-y1)*tdiff); */

	if (use_rates) {
	  rate = calc_rate(samps);
	}
	else
	  rate = 1;

	dur = (t2-t1);
	dur *= SR;
	end_samp = start_samp + dur;
	run_samps = (end_samp - start_samp);
	tdiff = (samps-start_samp)/run_samps;
	
	xvel = xvel*rate;
	yvel = yvel*rate;
	xchange = xvel*tdiff;
	ychange = yvel*tdiff;
	
	/* xchange = ((xvel+(rate*xvel))/2)*tdiff; */
	/* ychange = ((yvel+(rate*yvel))/2)*tdiff; */

	newx = x1 + xchange;
	newy = y1 + ychange;

	mag1 = sqrt(pow((newx-x1),2) + pow((newy-y1),2));
	mag2 = sqrt(pow((x2-x1),2) + pow((y2-y1),2));

	if (mag1 > mag2) {
	  cur_point++;
#ifdef DBUG
	   printf("cur_point %d\n",cur_point);
	  printf("old_point %d\n",old_point);
#endif
	   /* update things accordingly */
	  if ((use_rates) && ((cur_point+1) >= max_points)){
		for(i=0;i<max_points;i++) {
		  aud_locs[i].time += cycle;
		}
		cur_point = 0;
	  }
	  aud_locs[cur_point].startsamp = samps;
	}

	in_point->x = x1+xchange;
	in_point->y = y1+ychange;

#ifdef DBUG
	printf("[%d] %2.2f\t",samps,rate);
	printf("(%2.2f,%2.2f)\n",in_point->x,in_point->y);
#endif
	
  }
  else {
	in_point->x = x1;
	in_point->y = y1;
  }
}

void calc_amps(pt point) {
  int i,j;
  double amp,avg_dist, dist,decay_factor;
  
  decay_factor = 1;  /* Should experiment with that */
  avg_dist = tot_dist/n_spk;

  for(i=0;i<n_spk;i++) {
	dist = calc_dist(spk_locs[i],point);
	amp = (1 + (avg_dist-dist))/(avg_dist+1);
	if (amp < 0)
	  amp = 0;
     out_chan_amp[i] = amp;
#ifdef DBUG
     printf("out_chan_amp[%d] = %2.2<f\n",i,amp);
#endif
  }	  
}

void update_amps(long samps) {
  pt new_point;
  calc_loc(samps, &new_point);
  calc_amps(new_point);
}

int profile() {
  UG_INTRO("speakerloc",speakerloc);
  UG_INTRO("speakerloc_p",speakerloc_p);
  UG_INTRO("path",path);
  UG_INTRO("path_p",path_p);
  UG_INTRO("rates",rates);
}

// end funcs.c --------------------------------------------------------------

MIXN::MIXN() : Instrument()
{
  in = NULL;
}

MIXN::~MIXN()
{
  delete [] in;
}

int MIXN::init(float p[], int n_args)
{
  // p0 = outsk
  // p1 = insk
  // p2 = duration (-endtime)
  // p3 = inchan
  // p4-n = channel amps
  // we're (still) stashing the setline info in gen table 1

  int i;
  Instrument::init(p, n_args);
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

  return(this->mytag);
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
  inst->set_instnum("MIXN");
  return inst;
}

void
rtprofile()
{
  RT_INTRO("MIXN",makeMIXN);
}

