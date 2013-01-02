#include <ugens.h>
#include <sfheader.h>
#include <stdio.h>
#include <math.h>

extern	SFHEADER	sfdesc[NFILES];
extern float SR();

/*  rotate -- a pitch-shifting instrument based upon the idea
*	of old rotating tape-head pitch shifters
*
*  p0 = output skip
*  p1 = input skip
*  p2 = duration
*  p3 = amplitude multiplier
*  p4 = pitch shift up or down (oct.pc)
*  p5 = window size
*  p6 = input channel number
*  p7 = stereo spread (0-1) [optional]
*  assumes function table 1 is the amplitude envelope
*  assumes function table 2 is the window envelope
*	<usually a hanning window -- use "makegen(2, 25, 1000, 1)">
*
*/

double
rotate(float p[], int n_args)
{
	float samplenum1,samplenum2,x,interval;
	float val1,val2,amp=0.0,in[2],out[2];
	int nsamps,i,j,k,off,reinit,chans,inchan;
	int octpart;
	float pcpart;
	float amptabs[2];
	double *amptable, *wintable;
	int wlen;
	int skip, cdown;

	getsetnote(p[1], p[2], 0);
	nsamps = setnote(p[0], p[2], 1);

	amptable = floc(1);
	if (amptable) {
		int alen = fsize(1);
		tableset(SR(), p[2], alen, amptabs);
	}
	else
		rtcmix_advise("rotate", "Setting phrase curve to all 1's.");
	
	wintable = floc(2);
	if (wintable == NULL)
		die("rotate", "You haven't made the window envelope (table 2).");
	wlen = fsize(2);

	octpart = (int)p[4] * 12;
	pcpart = (p[4] * 100.0) - (float)(octpart*100);
	interval =  pow(2.0, ((float)octpart + pcpart)/12.0) - 1.0;

	reinit = p[5] * SR();
	off = reinit/2;
	k = off;
	chans = sfchans(&sfdesc[1]); 
	inchan = p[6];
	skip = SR()/(float)resetval;
	cdown = 0;
	j = 0;
	for(i = 0; i < nsamps; i++) {
		while (!cdown--) {
			if (amptable)
				amp = tablei(i, amptable, amptabs) * p[3];
			else
				amp = p[3];
			cdown = skip;
			}

		j = (j+1) % reinit;
		k = (k+1) % reinit;
		
		samplenum1 = (float)i + (float)j * interval;
		if(!GETSAMPLE(samplenum1, in, 0)) break;
		x = wintable[(int)(((float)j/reinit) * wlen)];
		val1 = in[inchan] * x;

		samplenum2 = (float)(i) + (float)(k-off) * interval;
		if(!GETSAMPLE(samplenum2, in, 0)) break;
		x = wintable[(int)(((float)k/reinit) * wlen)];
		val2 = in[inchan] * x;

		out[0] = (val1 + val2) * amp;
		if (chans > 1) {
			out[1] = (1.0 - p[7]) * out[0];
			out[0] *= p[7];
			}
		
		ADDOUT(out, 1);
		}

	endnote(1);

	return 0.0;
}


int profile()
{
	UG_INTRO("rotate",rotate);

	return 0;
}

