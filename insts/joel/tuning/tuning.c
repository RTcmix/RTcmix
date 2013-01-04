#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ugens.h>
#include <math.h>
  
#define DEBUG

/*	libtuning: custom temperaments in oct.pc notation
	-------------------------------------------------

	this library has predefined temperaments, which you can access in
	oct.pc notation. you can also define your own custom tunings.

	predefined tunings:
	-------------------
	pythag: Pythagorean Diatonic Scale (7 notes)
	meantone: Chromatic Quarter Comma Mean Tone Temperament (12 pitches)
	just: 5-Limit Just Intonation (12 pitches)
	young: LaMonte Young's tuning from _The Well Tempered Piano_ (12 pitches)
	partch: Harry Partch's infamous 11-limit scale (43 pitches)
	eqtemp: Equal Temperament* (12 notes)
		* I know we already have equal temperament in RTcmix, but this version
			allows you to change the diapason, in case, for instance, you
			want to tune to A=442. It also keeps things consistent if you want
			to move back and forth between various tuning and maintain the same
			diapason.
	
	you can change the diapason (the tuning fundamental) with the diapason command.
	
	This:
	
	diapason(440, 8.00)
	
	declares 440 Hz to be oct.pc 8.00 in the new tuning. (In essence, we have
	transposed 8.00 from middle C to the A above it.)
	
	if the first value of diapason is less than 18, it is interpreted as an
	RTcmix oct.pc value. So:

	diapason (8.00, 8.00)
	
	declares equal-tempered middle C (261.626 Hz) to be 8.00. This is the default
	value, so it's not strictly necessary to use the diapason command before using
	a tuning command.

	Heinrich Taube's _Notes from the Metalevel_ was a great help in designing and
	implementing this library.

Joel Matthys
19 November 2010
*/

double partchScale[44] = { 1.0, 81.0/80, 33.0/32, 21.0/20, 16.0/15, 12.0/11,
   11.0/10, 10.0/9, 9.0/8, 8.0/7, 7.0/6, 32.0/27, 6.0/5., 11.0/9, 5.0/4, 14.0/11,
   9.0/7, 21.0/16, 4.0/3, 27.0/20, 11.0/8,  7.0/5, 10.0/7, 16.0/11, 40.0/27, 3.0/2,
   32.0/21, 14.0/9, 11.0/7, 8.0/5, 18.0/11, 5.0/3, 27.0/16, 12.0/7, 7.0/4, 16.0/9,
   9.0/5, 20.0/11, 11.0/6, 15.0/8, 40.0/21, 64.0/33, 160.0/81, 2.0};
   
double justScale[13] = { 1.0, 16.0/15, 9.0/8, 6.0/5, 5.0/4, 4.0/3, 45.0/32, 3.0/2, 8.0/5, 5.0/3, 9.0/5, 15.0/8, 2.0 };

double pythagScale[9] = {1.0, 9.0/8, 81.0/64, 4.0/3, 3.0/2, 27.0/16, 243/128.0, 2.0 };

double youngScale[13] = {1.0, 567.0/512, 9.0/8, 147.0/128, 1323.0/1024, 21.0/16, 189.0/128, 3.0/2, 49.0/32, 441.0/256, 7.0/4, 63.0/32, 2.0};

double meanScale[13] = {1.0, 1.069984, 1.118034, 1.196279, 1.25, 1.337481, 1.397542, 1.495349, 1.5625, 1.671851, 1.746928, 1.869186, 2.0};

double myScale[64];
int myScaleLength = 0;

double diap = 261.625565301; // middle C
double octaveOffset = 8.0;

inline double myMap(double inval, double low0, double high0, double low1, double high1)
{
	double normVal = (inval-low0)/(high0-low0);
	return low1 + normVal*(high1-low1);
}

double parse(double note, double *whichScale, int elements)
{
   double oct = floor(note);   
   double pc = 100.0 * fmod(note,1.0);
   double frac = fmod(pc,1.0);
   double octaveSize = whichScale[elements];
   double myDiap = diap / pow(octaveSize, octaveOffset);
   if (100.0-pc<=0.000001) 
   {
      return pow(octaveSize,oct+1) * myDiap; // this is to sort out a nasty bug in RTcmix
      // that comes from a precision difference between double and float.
   }
   int basepitch = (int)floor(pc);
   while (basepitch>=elements)
   {
      oct+= 1.0;
      basepitch = basepitch - elements;
   }
   double pitchratio = whichScale[basepitch];
   double nextratio = whichScale[basepitch+1];
   double interp_ratio = myMap(frac,0.0,1.0,pitchratio,nextratio);
   double baseFreq = pow(octaveSize,oct) * myDiap;
   return baseFreq * interp_ratio;
}

double m_eqtemp(float p[], int n_args, double pp[])
{
	double note = pp[0];
	double oct = floor(note);   
	double basepitch = 100.0 * fmod(note,1.0);
	double myDiap = diap / pow(2.0, octaveOffset);
   if (100.0-basepitch<=0.000001) 
   {
      return pow(2.0,oct+1) * myDiap; // this is to sort out a nasty bug in RTcmix
      // that comes from a precision difference between double and float.
   }
   while (basepitch>=12.0)
   {
      oct+= 1.0;
      basepitch = basepitch - 12;
   }
   double baseFreq = pow(2.0,oct) * myDiap;
   return baseFreq * pow(2.0,basepitch/12.0);
}

double m_create_scale(float p[], int n_args, double pp[])
{
   myScaleLength = n_args-1;
   int ii;
   for (ii=0; ii<n_args; ii++)
   {
      myScale[ii] = pp[ii];
   }
   return 0;
}

double m_myscale(float p[], int n_args, double pp[])
{
   return parse(pp[0],myScale,myScaleLength);
}

double m_diapason(float p[], int n_args, double pp[])
{
   diap = (pp[0]<18.0) ? cpspch(pp[0]) : pp[0];
   octaveOffset = (n_args>1) ? pp[1] : 8.0;
   return 0;
}

double m_partch(float p[], int n_args, double pp[])
{
   return parse(pp[0],partchScale,43);
}

double m_young(float p[], int n_args, double pp[])
{
   return parse(pp[0],youngScale,12);
}

double m_just(float p[], int n_args, double pp[])
{
   return parse(pp[0],justScale,12);
}

double m_pythag(float p[], int n_args, double pp[])
{
   return parse(pp[0],pythagScale,7);
}

double m_mean(float p[], int n_args, double pp[])
{
   return parse(pp[0],meanScale,12);
}

#ifndef MAXMSP
/* -------------------------------------------------------------- profile --- */
int
profile()
{
   UG_INTRO("diapason", m_diapason);
   UG_INTRO("partch", m_partch);
   UG_INTRO("young", m_young);
   UG_INTRO("just", m_just);
   UG_INTRO("pythag", m_pythag);
   UG_INTRO("meantone", m_mean);
   UG_INTRO("eqtemp", m_eqtemp);
   UG_INTRO("create_scale", m_create_scale);
   UG_INTRO("myscale", m_myscale);
   return 0;
}
#endif
