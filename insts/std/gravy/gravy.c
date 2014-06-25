#include <ugens.h>
#include <sfheader.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <stdlib.h>

extern SFHEADER      sfdesc[NFILES];
extern float SR();

/* gravy -- a time and pitch-shifting instrument
*  	(modified 11/89 by D.S. to use parabolic interpolation)
*
*  p0 = output skip
*  p1 = input skip
*  p2 = duration (on input)
*  p3 = window size (in seconds)
*  p4 = amplitude multiplier
*  p5 = length multiplier (of original)
*  p6 = transposition interval (oct.pc)
*  p7 = input channel [optional]
*  p8 = stereo spread (0-1) [optional]
*  assumes function table 1 contains the window envelope
*
*/

float transpose(float);
double interp(double, double, double, double);

double
gravy(float p[], int n_args)
{
	int insamps,outsamps,winsamps,inmove;
	int inshift,outshift,inchans,outchans,inchan;
	float *inarr,*outarr,skip,val=0.0,wintabs[2];
	double *winfunc;
	int winsize;
	double vold,old,new,frac,count;
	long i,j,k,m,intt,lgc,n,ncuts;

	winsamps = setnote(p[1],p[3],0);
	setnote(p[0],p[2],1);
	ncuts = (p[2] * p[5])/(p[3]/2.0) + 1.0;
	p[6] = transpose(p[6]);

	winfunc = floc(1);
	if (winfunc == NULL)
		die("gravy", "You haven't made a window envelope function (table 1).");
	winsize = fsize(1);
	tableset(SR(), p[3], winsize, wintabs);

	inchans = sfchans(&sfdesc[0]);
	outchans = sfchans(&sfdesc[1]);

	insamps = (p[3] * SR() * p[6] + 1.0) * (float)inchans;
	if ( (inarr = (float *)malloc(insamps*FLOAT)) == NULL )
		die("gravy", "Error allocating input array... sorry!");
	
	outsamps = winsamps * (float)outchans;
	if ( (outarr = (float *)malloc(outsamps*FLOAT)) == NULL )
		die("gravy", "Error allocating output array... sorry!");

	outshift = -(winsamps/2);
	inmove = (1.0/p[5]) * (winsamps/2);
	inshift = -((insamps/inchans) - inmove);

	skip = SR()/(float)resetval;
	inchan = p[7];

	for (n = 0; n < ncuts; n++) {
		bgetin(inarr,0,insamps);
		j = k = m = 0;
		vold = inarr[k+inchan];
		k += inchans;
		old = inarr[k+inchan];
		k += inchans;
		new = inarr[k+inchan];
		k += inchans;
		intt = lgc = 0;
		frac = count = 0;

		for (i = 0; i < winsamps; i++) {
			while(!j--) {
				val = tablei(i, winfunc, wintabs) * p[4];
				j = skip;
				}
			while (intt > lgc) {
				vold = old;
				old = new;
				new = inarr[k+inchan];
				k += inchans;
				lgc++;
				}
			outarr[m] = (float) interp(vold, old, new, frac) * val;
			if (outchans > 1) {
				outarr[m+1] = (1.0 - p[8]) * outarr[m];
				outarr[m] *= p[8];
				}
			m += outchans;

			count += p[6];
			intt = count;
			frac = count - (float) intt;
			}

		baddout(outarr,1,outsamps);
		inrepos(inshift,0);
		outrepos(outshift,1);
		}
	endnote(1);

	return 0.0;
}


float transpose(float num)
{
	int octave;
	float fract,mult;
	double pow();

	octave = num;
	fract = (num - (float)octave)/.12;
	mult = (float)pow(2.0, (double)(fract + (float)octave));
	return(mult);
}

double interp(double y0, double y1, double y2, double t)
{
    register double hy2, hy0, a, b, c;
    
    a = y0;
    hy0 = y0/2.0;
    hy2 =  y2/2.0;
    b = (-3.0 * hy0) + (2.0 * y1) - hy2;
    c = hy0 - y1 + hy2;

    return(a + b*t + c*t*t);
}


int
profile()
{
	UG_INTRO("gravy",gravy);
	return 0;
}

