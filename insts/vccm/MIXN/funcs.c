#define xDBUG
#include <stdlib.h>
#include <stdio.h>
#include <globals.h>
#include <math.h>
#include <ugens.h>
#include "funcs.h"


int num_points;
float out_chan_amp[MAXBUS]; /* Used by inst */
double tot_dist;
int n_spk;
Bool use_path;
Bool use_rates;
double cycle;  /* Length of 1 iteration ... last path time */

rfact ratefs[MAXBUS];
int max_rates;

pt spk_locs[MAXBUS]; /* Do we have a variable for this? */
loc_slot *cur_slot=NULL;
loc_slot *aud_locs=NULL;
loc_slot *last_slot=NULL;
accel_slot *accel_curve=NULL;

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
	return 0.0;
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
	return 0.0;
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
	return 0.0;
}

double path(float p[], int n_args, double pp[]) {
	int i,j;
	double x,y,r,a,time,x1,x2,y1,y2,xvel,yvel,t1,t2,startsamp;
	loc_slot *tl_slot;

	if (p[0] != 0) {
		fprintf(stderr,"WARNING:  path start not = 0!\n");
	}
	if (n_args%3 != 0) {
		fprintf(stderr,"WARNING:  path wrong number of arguments\n");
	}
  
	i=j=0;
	while(i<n_args) {
		tl_slot = (loc_slot*) malloc(sizeof(loc_slot));
		tl_slot->next = NULL;

		time = p[i++];
		x = p[i++];
		y = p[i++];

		tl_slot->point.x = x;
		tl_slot->point.y = y;
		tl_slot->time = time;
		tl_slot->xvel = 0;
		tl_slot->yvel = 0;
		tl_slot->startsamp = 0;
		
		if (!aud_locs) {
			aud_locs = tl_slot;
			last_slot = tl_slot;
			cur_slot = tl_slot;
		}
		else {
			last_slot->next = tl_slot;
			last_slot = tl_slot;
		}

#ifdef DBUG
		printf("Point [%d] set:  %2.2f,%2.2f,%2.2f\n",j,time,x,y);
#endif
	}
	i=0;
	startsamp = 0;
	t2=0; /* to get rid of warning message */
	tl_slot = aud_locs;
	while(tl_slot->next) {
		x1 = tl_slot->point.x;
		y1 = tl_slot->point.y;
		t1 = tl_slot->time;

		tl_slot = tl_slot->next;
		x2 = tl_slot->point.x;
		y2 = tl_slot->point.y;
		t2 = tl_slot->time;
		xvel = (x2-x1)/(t2-t1);
		yvel = (y2-y1)/(t2-t1);
		startsamp += (t2-t1)*SR;
		tl_slot->xvel = xvel;
		tl_slot->yvel = yvel;
		tl_slot->startsamp = startsamp;
		tl_slot = tl_slot->next;
#ifdef DBUG
		printf("Point vel [%d] set:  %2.2f,%2.2f,%2.2f\n",i,xvel,yvel,startsamp);
#endif
	}
	cycle = t2;
	use_path = YES;
	return 0.0;
}

/* change to support xvel,yvel */

double path_p(float p[], int n_args, double pp[]) {
	int i,j;
	double x,y,r,a,time,x1,x2,y1,y2,xvel,yvel,t1,t2,startsamp;
	loc_slot *tl_slot=NULL;

	if (p[0] != 0) {
		fprintf(stderr,"WARNING:  path_p start not = 0!\n");
	}
	if (n_args%3 != 0) {
		fprintf(stderr,"WARNING:  path_p wrong number of arguments\n");
	}
  
	i=j=0;
	while(i<n_args) {
		tl_slot = (loc_slot *)malloc(sizeof(loc_slot));
		time = p[i++];
		r = p[i++];
		a = p[i++];

		a = 2*PI*(a/360);  /* convert to radians */

		x = r*(cos(a));
		y = r*(sin(a));

		tl_slot->point.x = x;
		tl_slot->point.y = y;
		tl_slot->time = time;

		if (!aud_locs) {
			aud_locs = tl_slot;
			last_slot = tl_slot;
			cur_slot = tl_slot;
		}
		else {
			last_slot->next = tl_slot;
			last_slot = tl_slot;
		}
		printf("Point set:  %2.2f,%2.2f\n",x,y);

	}
	i=0;
	startsamp = 0;
	tl_slot = aud_locs;
	while(tl_slot->next) {
		x1 = tl_slot->point.x;
		y1 = tl_slot->point.y;
		t1 = tl_slot->time;

		tl_slot = tl_slot->next;
		x2 = tl_slot->point.x;
		y2 = tl_slot->point.y;
		t2 = tl_slot->time;
		xvel = (x2-x1)/(t2-t1);
		yvel = (y2-y1)/(t2-t1);
		startsamp += (t2-t1)*SR;
		tl_slot->xvel = xvel;
		tl_slot->yvel = yvel;
		tl_slot->startsamp = startsamp;
		printf("Vel set (%2.2f, %2.2f):  %2.2f,%2.2f,%2.2f\n",x2,y2,xvel,yvel,startsamp);
	}
	use_path = YES;
	printf("path_p() exit\n");
	return 0.0;
}

double calc_rate(long samps) {
	return 0.0;
}

void calc_loc(long samps, pt *in_point) {
	int i;
	double slope,x1,x2,y1,y2,t1,t2,tdiff,ychange,xchange,start_samp,end_samp,run_samps,xdiff,ydiff,newx,newy,mag1,mag2;
	double rate=1,rate2=1;
	double xvel,yvel,dur;
	loc_slot *next_slot;

	x1 = cur_slot->point.x;
	y1 = cur_slot->point.y;
	t1 = cur_slot->time;
	start_samp = cur_slot->startsamp;
	next_slot = cur_slot->next;

	printf("cur_slot:  %2.2f,%2.2f\n",cur_slot->point.x,cur_slot->point.y);
	if (next_slot) {
		printf("next_slot:  %2.2f,%2.2f\n",next_slot->point.x,next_slot->point.y);
		x2 = next_slot->point.x;
		y2 = next_slot->point.y;
		t2 = next_slot->time;
		xvel = next_slot->xvel;
		yvel = next_slot->yvel;

		printf("v = %2.2f,%2.2f\n",xvel,yvel);

		/* tdiff = (samps-start_samp)/run_samps; */
		/* xchange = ((x2-x1)*tdiff); */
		/* ychange = ((y2-y1)*tdiff); */

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
			cur_slot = cur_slot->next;
#ifdef DBUG
			printf("cur_point %d\n",cur_point);
			printf("old_point %d\n",old_point);
#endif
			cur_slot->startsamp = samps;
		}

		printf("mags: %2.2f,%2.2f\n",mag1,mag2);

		in_point->x = x1+xchange;
		in_point->y = y1+ychange;

#ifdef xDBUG
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
	return 0;
}
