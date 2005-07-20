#define xDBUG 1
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ugens.h>
#include <bus.h>
#include "mixn_structs.h"
#include "funcs.h"

extern float SR();

loc *aud_locs;
pt *spk_locs;
rfact *ratefs;
int num_rates;
int cur_rate;
int cur_point;
int num_points;
float *out_chan_amp; /* Used by inst */
double tot_dist;
int n_spk;
Bool use_path;
Bool use_rates;
double cycle;  /* Length of 1 iteration ... last path time */

/* calc_dist ------------------------------------------------------------------------- */

double calc_dist(pt p1, pt p2) {
  double ret_dist;
  double x1,x2,y1,y2,a,b;

  x1 = p1.x;
  x2 = p2.x;
  y1 = p1.y;
  y2 = p2.y;
  
  a = (x2-x1);
  b = (y2-y1);

#ifdef DBUG2
  printf("calc_dist:  %2.1f,%2.1f,%2.1f,%2.1f\n",x1,y1,x2,y2);
#endif
  ret_dist = sqrt(pow(a,2) + pow(b,2));
 
  return ret_dist;
}

/* calc_total_dist ------------------------------------------------------------------- */

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

/* speakerloc ------------------------------------------------------------------------ */

double speakerloc(float p[], int n_args, double pp[]) {
  int i,j;

  i=j=0;
  n_spk=0;

  spk_locs = (pt *)malloc(MAXSPEAKS * sizeof(pt));
  
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
  return 0;
}

/* speakerloc_p ---------------------------------------------------------------------- */

double speakerloc_p(float p[], int n_args, double pp[]) {
  int i,j;
  double x,y,r,a;

  n_spk=0;
  i=j=0;
  
  spk_locs = (pt *)malloc(MAXSPEAKS * sizeof(pt));

  while(j<n_args) {
	r = p[j++];
	a = p[j++];

	a = 2*PI*(a/360);  /* convert to radians */

	x = r*(sin(a));
	y = r*(cos(a));

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
  return 0;
}

/* rates ---------------------------------------------------------------------------- */

double rates(float p[], int n_args, double pp[]) {
  int i,j;
  double s,time;
  double r1,r2,t1,t2,accel;

  num_rates=0;
  if (p[0] != 0) {
	fprintf(stderr,"WARNING:  rates start not = 0!\n");
  }
  if (n_args%2 != 0) {
	fprintf(stderr,"WARNING:  rates wrong number of arguments\n");
  }
  
  ratefs = (rfact *)malloc(MAXRATES * sizeof(rfact));

  i=j=0;
  while(i<n_args) {
	time = p[i++] * (double) SR();
	s = p[i++];
	ratefs[j].factor = s;
	ratefs[j].time = time;
	j++;
	num_rates++;
  }
  use_rates = YES;
  i=0;
  while(i<num_rates-1) {
	r1 = ratefs[i].factor;
	r2 = ratefs[i+1].factor;
	t1 = ratefs[i].time;
	t2 = ratefs[i+1].time;
	accel = (r2-r1)/(t2-t1);
	ratefs[i+1].accel = accel;
	i++;
  }
  return 0;
}

/* path ------------------------------------------------------------------------------ */

double path(float p[], int n_args, double pp[]) {
  int i,j;
  double x,y,r,a,time,x1,x2,y1,y2,xvel,yvel,t1,t2;

  time = 0;
  num_points=0;
  if (p[0] != 0) {
	fprintf(stderr,"WARNING:  path start not = 0!\n");
  }
  if (n_args%3 != 0) {
	fprintf(stderr,"WARNING:  path wrong number of arguments\n");
  }
  
  i=j=0;

  aud_locs = (loc *)malloc(MAXLOCS * sizeof(loc));
  
  while(i<n_args) {
	time = p[i++] * (double)SR();
	x = p[i++];
	y = p[i++];

	aud_locs[j].point.x = x;
	aud_locs[j].point.y = y;
	aud_locs[j].time = time;
	aud_locs[j].atime = time;
	aud_locs[j].xvel = 0;
	aud_locs[j].yvel = 0;
#ifdef DBUG
     printf("Point [%d] set:  %2.2f,%2.2f,%2.2f\n",j,time,x,y);
#endif
     j++;
	num_points++;
  }
  i=0;
  while(i<num_points-1) {
	x1 = aud_locs[i].point.x;
	y1 = aud_locs[i].point.y;
	t1 = aud_locs[i].time;
	x2 = aud_locs[i+1].point.x;
	y2 = aud_locs[i+1].point.y;
	t2 = aud_locs[i+1].time;
	xvel = (x2-x1)/(t2-t1);
	yvel = (y2-y1)/(t2-t1);
	aud_locs[i+1].xvel = xvel;
	aud_locs[i+1].yvel = yvel;
#ifdef DBUG
     printf("Point vel [%d] set:  %2.2f,%2.2f\n",i,xvel,yvel);
#endif
     i++;
  }

  cycle = time;
  use_path = YES;
  return 0;
}

/* change to support xvel,yvel */

/* path_p ---------------------------------------------------------------------------- */

double path_p(float p[], int n_args, double pp[]) {
  int i,j;
  double x,y,r,a,time,x1,x2,y1,y2,xvel,yvel,t1,t2;

  if (p[0] != 0) {
	fprintf(stderr,"WARNING:  path_p start not = 0!\n");
  }
  if (n_args%3 != 0) {
	fprintf(stderr,"WARNING:  path_p wrong number of arguments\n");
  }
  
  num_points=0;
  time=0;
  i=j=0;

  aud_locs = (loc *)malloc(MAXLOCS * sizeof(loc));

  while(i<n_args) {
	time = p[i++] * (double) SR();
	r = p[i++];
	a = p[i++];

	a = 2*PI*(a/360);  /* convert to radians */

	x = r*(sin(a));
	y = r*(cos(a));


	aud_locs[j].point.x = x;
	aud_locs[j].point.y = y;
	aud_locs[j].time = time;
	aud_locs[j].atime = time;
	j++;
	num_points++;
  }
  i=0;
  while(i<num_points-1) {
	x1 = aud_locs[i].point.x;
	y1 = aud_locs[i].point.y;
	t1 = aud_locs[i].time;
	x2 = aud_locs[i+1].point.x;
	y2 = aud_locs[i+1].point.y;
	t2 = aud_locs[i+1].time;
	xvel = (x2-x1)/(t2-t1);
	yvel = (y2-y1)/(t2-t1);
	aud_locs[i+1].xvel = xvel;
	aud_locs[i+1].yvel = yvel;
#ifdef DBUG
     printf("Point [%d] set:  %2.2f,%2.2f,%2.2f,%2.2f (%f,%f)\n",
	    i,x1,y1,x2,y2,xvel,yvel);
#endif
     i++;
  }
  cycle = time;
  use_path = YES;
  return 0;
}

/* calc_rate ------------------------------------------------------------------------- */

double calc_rate(long samps) {
  double r1,r2,t1,t2,tdiff,rdiff,start_samp,run_samps,rate;
  double accel;

  r1 = ratefs[cur_rate].factor;
  t1 = ratefs[cur_rate].time;

  if ((cur_rate+1) < num_rates) {
	r2 = ratefs[cur_rate+1].factor;
	t2 = ratefs[cur_rate+1].time;
	accel = ratefs[cur_rate+1].accel;
	
	rdiff = accel * (samps-t1);

	if (samps > t2)
	  cur_rate++;

	rate = r1+rdiff;
  }
  else {
	rate = r1;
  }
  return rate;
}

/* calc_loc ------------------------------------------------------------------------- */

void calc_loc(long samps, pt *in_point) {
  int i;
  double slope,x1,x2,y1,y2,t1,t2,tdiff,ychange,xchange,start_samp,run_samps,xdiff,ydiff,newx,newy,mag1,mag2;
  double rate=1,rate2=1;
  double xvel,yvel,dur;
  int old_point;

  old_point = cur_point;

  x1 = aud_locs[cur_point].point.x;
  y1 = aud_locs[cur_point].point.y;
  if (use_rates)
    t1 = aud_locs[cur_point].atime;
  else
    t1 = aud_locs[cur_point].time;

#ifdef DBUG
	printf("[%ld] %2.2f\t",samps,rate);
	printf("(%2.2f,%2.2f)\t",x1,y1);
#endif

  if ((cur_point+1) < num_points) {
	x2 = aud_locs[cur_point+1].point.x;
	y2 = aud_locs[cur_point+1].point.y;
	t2 = aud_locs[cur_point+1].time;
	xvel = aud_locs[cur_point+1].xvel;
	yvel = aud_locs[cur_point+1].yvel;

#ifdef DBUG
     printf("v = %f,%f ->",xvel,yvel);
#endif

	if (use_rates) {
	  rate = calc_rate(samps);
	}
	else
	  rate = 1;

	xvel = xvel*rate;
	yvel = yvel*rate;

#ifdef DBUG
     printf("v (%f,%f) r (%f)",xvel,yvel,rate);
#endif

	xchange = xvel*(samps-t1);
	ychange = yvel*(samps-t1);
	
#ifdef DBUG
     printf("change (%f,%f)  ",xchange,ychange);
#endif

	newx = x1 + xchange;
	newy = y1 + ychange;

	mag1 = sqrt(pow((newx-x1),2) + pow((newy-y1),2));
	mag2 = sqrt(pow((x2-x1),2) + pow((y2-y1),2));

	if (mag1 > mag2) {

	  cur_point++;
	  /* update things accordingly */
	  if ((use_rates) && ((cur_point+1) >= num_points)){
		for(i=0;i<num_points;i++) {
		  aud_locs[i].time += cycle;
		}
		cur_point = 0;
	  }
	  aud_locs[cur_point].atime = samps;
#ifdef DBUG
	  printf("\ncur_point %d -> ",cur_point);
	  printf("old_point %d : num_points %d\n",old_point,num_points);
#endif

	}

	in_point->x = x1+xchange;
	in_point->y = y1+ychange;
  }
  else {
	in_point->x = x1;
	in_point->y = y1;
  }
#ifdef DBUG
	printf("\n[%ld] %2.2f\t",samps,rate);
	printf("(%2.2f,%2.2f)\n",in_point->x,in_point->y);
#endif
}

/* calc_amps ------------------------------------------------------------------------- */

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
#ifdef DBUG2
     printf("out_chan_amp[%d] = %2.2f\n",i,amp);
#endif
  }	  
}

/* update_amps ----------------------------------------------------------------------- */

void update_amps(long samps) {
  pt new_point;
  calc_loc(samps, &new_point);
  calc_amps(new_point);
}

/* profile --------------------------------------------------------------------------- */

int profile() {
  UG_INTRO("speakerloc",speakerloc);
  UG_INTRO("speakerloc_p",speakerloc_p);
  UG_INTRO("path",path);
  UG_INTRO("path_p",path_p);
  UG_INTRO("rates",rates);
  return 0;
}
