#include <ugens.h>
#include <sfheader.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <math.h>

extern SFHEADER      sfdesc[NFILES];
extern float SR();

/*
   sgran:
      0              start time of group
      1              duration of group
      2              amplitude
      3              beginning grain rate (time in seconds btw. grains)
      4              ending grain rate

      amount of variation in rate: (percentage of grain rate)
      5-8            zero: lo, average, hi, tightness (0-1, is 0-100%)
      9-12           one: lo, average, hi, tightness (0-1, is 0-100%)

      average duration:
      13-16          zero: lo, average, hi, tightness
      17-20          one: lo, average, hi, tightness

      location:
      21-24          zero: lo, average, hi, tightness
      25-28          one: lo, average, hi, tightness

      frequency band:
      29-32          zero: lo, average, hi, tightness
                     (if p29 < 0, noise is the input)
      33-36          one: lo, average, hi, tightness

      37             random seed (integer) [optional]

                 *       *       *

   functions: (stt variation changes are linear)

      1              overall envelope (or setline)
                     (was grain envelope prior to 12 June, 1999)

      shape of change (usually linear for all shapes):
      2              grain density
      3              grain duration
      4              grain location
      5              grain frequency

      6              oscillator waveform

      8              grain envelope
*/

double
sgran(float p[], int n_args)
{
	long n,bgrainsamps,bgraindist,bgrainslide;
	long i,nsamps,gstt_var,count;
	long egrainsamps,egraindist,egrainslide;
	long grainsamps,grainslide,graindistdiff;
	float si=0.0,phase,val=0.0,amp,out[2],chb,freq;
	float tabs[2],tab[2],tab2[2],tab3[2],tab4[2],tab5[2];
	double *array,*wave,*envel,*rate_shape,*dur_shape,*loc_shape,*freq_shape;
	float gdist_inc;
	double gstt_per,lo,mid,hi,ti,slodiff,smiddiff,shidiff,stidiff;
	double dlodiff,dmiddiff,dhidiff,dtidiff;
	double llodiff,lmiddiff,lhidiff,ltidiff;
	double flodiff,fmiddiff,fhidiff,ftidiff;
	int len,j,z,chans,randflag=0;

#ifdef MAXMSP
	int outrepos();
#endif
	float rrand();
	void srrand();
	double prob();

	if (p[37] > 0)
		srrand(p[37]);
   else
		srrand(3);

	nsamps = setnote(p[0],p[1],1); /* set file position */

	array = floc(1);             /* used to be setline  -JGG */
	if (array) {
		int amplen = fsize(1);
		tableset(SR(), p[1], amplen, tabs);
	}
	else
		rtcmix_advise("sgran", "Setting phrase curve to all 1's.");

	wave = floc(6); /* finds starting loc. of waveform */
	if (wave == NULL)
		die("sgran", "You haven't made the oscillator waveform (table 6).");
	len = fsize(6); /* length of playing waveform function */

	envel = floc(8);  /* NOTE: used to be floc(1), now stolen by setline  -JGG */
	if (envel == NULL)
		die("sgran", "You haven't made the grain envelope (table 8).");
	/* NOTE: fsize(8) called in loop below */

	bgrainsamps = grainsamps = p[14] * SR();
	bgraindist = p[3] * SR();
	bgrainslide = grainslide = bgraindist - bgrainsamps;

	egrainsamps = p[18] * SR();
	egraindist = p[4] * SR();
	egrainslide = egraindist - egrainsamps;

	graindistdiff = egraindist - bgraindist;

	rate_shape = floc(2);
	if (rate_shape == NULL)
		die("sgran", "You haven't made the grain density function (table 2).");
	tableset(SR(), p[1]-p[4],fsize(2),tab2);

	dur_shape = floc(3);
	if (dur_shape == NULL)
		die("sgran", "You haven't made the grain duration function (table 3).");
	tableset(SR(), p[1]-p[4],fsize(3),tab3);

	loc_shape = floc(4);
	if (loc_shape == NULL)
		die("sgran", "You haven't made the grain location function (table 4).");
	tableset(SR(), p[1]-p[4],fsize(4),tab4);

	freq_shape = floc(5);
	if (freq_shape == NULL)
		die("sgran", "You haven't made the grain frequency function (table 5).");
	tableset(SR(), p[1]-p[4],fsize(5),tab5);

	slodiff = (double)(p[9]-p[5])/nsamps; /* get stt zero/one differences */
	smiddiff = (double)(p[10]-p[6])/nsamps;
	shidiff = (double)(p[11]-p[7])/nsamps;
	stidiff = (double)(p[12]-p[8])/nsamps;

	dlodiff = (double)(p[17]-p[13]); /*get dur zero/one differences */
	dmiddiff = (double)(p[18]-p[14]);
	dhidiff = (double)(p[19]-p[15]);
	dtidiff = (double)(p[20]-p[16]);

	llodiff = (double)(p[25]-p[21]); /*get loc zero/one differences */
	lmiddiff = (double)(p[26]-p[22]);
	lhidiff = (double)(p[27]-p[23]);
	ltidiff = (double)(p[28]-p[24]);
	chb = p[21];

	if (p[29] < 0) 		/* if negative, noise is the input */
		randflag = 1;

	flodiff = (double)(p[33]-p[29]); /*freq zero/one differences */
	fmiddiff = (double)(p[34]-p[30]);
	fhidiff = (double)(p[35]-p[31]);
	ftidiff = (double)(p[36]-p[32]);

	z = 2;

	chans = sfchans(&sfdesc[1]); /* get file number of channels */
	amp = p[2];
	gstt_var = 0;
	count = 0;

	j = 0; /* "branch once a cycle" loop for amp envelope */

	for(i = 0; i < nsamps; i++) {
		count++;
		phase = 0;
		tableset(SR(), grainsamps/SR(),fsize(8),tab);
		if(!randflag) {
			lo = p[29] + flodiff*tablei(i,freq_shape,tab5);
			mid = p[30] + fmiddiff*tablei(i,freq_shape,tab5);
			hi = p[31] + fhidiff*tablei(i,freq_shape,tab5);
			ti = p[32] + ftidiff*tablei(i,freq_shape,tab5);
			lo = (lo > mid) ? mid : lo;
			hi = (hi < mid) ? mid : hi;
			ti = (ti < 0) ? 0 : ti; 
			freq = prob(lo, mid, hi, ti);
			si = freq * (float)len/SR(); 
		}

/*
	fprintf(stderr,"i: %ld, grainsamps: %ld, grainslide: %ld\n",i,grainsamps,grainslide);
*/

		for (n = 0; n < grainsamps; n++) {
			while (!j--) {   /* branch in here when j reaches 0 */
				float tmp = 1.0;
				if (array)
					tmp = tablei(i,array,tabs);
				val = amp * tablei(n,envel,tab) * tmp;
				j = ((grainsamps-n) > z) ? z : (grainsamps-n);
			}
			if(randflag)
				out[0] = rrand()*val;
			else
				out[0] =  oscili(val,si,wave,len,&phase);

			if (chans > 1) { /* stereo */
				out[1] = (1.0 - chb) * out[0];
				out[0] *= chb;
			}
			ADDOUT(out,1);
		}

		if((i+grainslide+gstt_var+grainsamps) < 0) {
			outrepos((grainslide),1);
			i += grainsamps;
			i += grainslide;
		}	
		else {
			outrepos((grainslide+gstt_var),1);
			i += grainsamps;
			i += grainslide;
			i += gstt_var;
		}

		lo = p[13] + dlodiff*tablei(i,dur_shape,tab3);
		mid = p[14] + dmiddiff*tablei(i,dur_shape,tab3);
		hi = p[15] + dhidiff*tablei(i,dur_shape,tab3);
		ti = p[16] + dtidiff*tablei(i,dur_shape,tab3);
		lo = (lo > mid) ? mid : lo;
		hi = (hi < mid) ? mid : hi;
		ti = (ti < 0) ? 0 : ti; 
		grainsamps = (long)(prob(lo, mid, hi, ti)*SR());


		/*	get percentage to vary next stt of grain */
		lo = p[5] + slodiff*i;
		mid = p[6] + smiddiff*i;
		hi = p[7] + shidiff*i;
		ti = p[8] + stidiff*i;
		lo = (lo > mid) ? mid : lo;
		hi = (hi < mid) ? mid : hi;
		ti = (ti < 0) ? 0 : ti; 
		gstt_per = prob(lo, mid, hi, ti);
		gstt_var = (long)(gstt_per*(grainsamps+grainslide)); 

/* calculate grainslide */
		gdist_inc = tablei(i,rate_shape,tab2);
		grainslide = (float)bgraindist + (float)graindistdiff*gdist_inc - grainsamps;

		lo = p[21] + llodiff*tablei(i,loc_shape,tab4);
		mid = p[22] + lmiddiff*tablei(i,loc_shape,tab4);
		hi = p[23] + lhidiff*tablei(i,loc_shape,tab4);
		ti = p[24] + ltidiff*tablei(i,loc_shape,tab4);
		lo = (lo > mid) ? mid : lo;
		hi = (hi < mid) ? mid : hi;
		ti = (ti < 0) ? 0 : ti; 
		chb = prob(lo, mid, hi, ti);
	}
	printf("\n%ld grains\n",count);
	endnote(1);

	return(0.0);
}


double prob(low,mid,high,tight)  
double low, mid, high, tight;      /* Returns a value within a range
				   close to a preferred value 
	
			   tightness: 0 max away from mid
				      1 even distribution
				      2+amount closeness to mid
				      no negative allowed */
{
	int repeat;
	double range, num, sign;
	repeat = 0;
	do {
	  	if ((high-mid) > (mid-low))    
			range = high-mid;
	    	else  
			range = mid-low;
	  	if (rrand() > 0.)  
			sign = 1.; 
		else  sign = -1.;
	  	num = mid + sign*(pow((rrand()+1.)*.5,tight)*range);
	  	if (num < low || num > high)  
			repeat++;
	    	else  
			repeat = 0;
	} while(repeat > 0);
	return(num);
}


int
profile()
{
	UG_INTRO("sgran",sgran); 
	return 0;
}

