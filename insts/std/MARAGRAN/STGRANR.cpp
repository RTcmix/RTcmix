#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "STGRANR.h"
#include <rt.h>
#include <rtdefs.h>
#include <math.h>

// realtime sampling stochastic grain maker based on stgran, takes mono/stereo input
// Mara Helmuth March 20, 2004


inline float interp(float y0, float y1, float y2, float t)
{
   float hy2, hy0, a, b, c;

   a = y0;
   hy0 = y0 / 2.0f;
   hy2 = y2 / 2.0f;
   b = (-3.0f * hy0) + (2.0f * y1) - hy2;
   c = hy0 - y1 + hy2;

   return (a + b * t + c * t * t);
}


STGRANR::STGRANR() : Instrument()
{
	in = NULL;
	branch = 0;
        grainoverlap = totalgrains = 0;
   
        incount = 1;
        counter = 0.0;
        get_frame = 1;

        /* clear sample history */
        oldersig[0] = oldersig[1] = 0.0;
        oldsig[0] = oldsig[1] = 0.0;
        newsig[0] = newsig[1] = 0.0;
}


STGRANR::~STGRANR()
{
	delete [] in;
}

int STGRANR::init(double p[], int n_args)
{
/*  p0=start_time
*  p1=input start time
*  now p2 was p1=duration
*  p3=amplitude
*  p4=rate, p5-8 ratevar
*  p9-12 duration
*  p13-16 location
*  p17-20 transposition
*  p21 granlayers (not in use)
*  p22 seed
*  makegen slot 1 is amp env, makegen slot 2 is grain env
*/
	starttime = p[0];
	inskip = p[1];
	evdur = p[2];
	if (evdur < 0.0)
		evdur = -evdur - inskip;

	if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE; // no input
	if (rtsetoutput(starttime, evdur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("STGRANR", "Can't handle more than 2 output channels.");

	amp = p[3];

	inframe = RTBUFSAMPS;
        
	amptable = floc(1);
	if (amptable) {
		alen = fsize(1);
		tableset(SR, evdur, alen, tabs);
	}
	else
		rtcmix_advise("STGRANR", "Setting phrase curve to all 1's.");
	grenvtable = floc(2);
	if (grenvtable) {
		grlen = fsize(2);
	}
	else
		rtcmix_advise("STGRANR", "Setting grain envelope to all 1's.");
        
	aamp = amp;
        
	skip = (int)(SR/(float)resetval);

	rate = p[4]; 
	ratevarlo  = (double)p[5];
	ratevarmid  = (double)p[6]; if (ratevarmid < ratevarlo ) ratevarmid = ratevarlo;
	ratevarhi  = (double)p[7]; if (ratevarhi < ratevarmid ) ratevarhi = ratevarmid;
	ratevarti  = (double)p[8];
	durlo  = (double)p[9];
	durmid  = (double)p[10]; if (durmid < durlo ) durmid = durlo;
	durhi  = (double)p[11]; if (durhi < durmid ) durhi = durmid;
   if ( durhi > rate ) {
 		die("STGRANR", "Grain durations %f sec. are larger than rate: %f seconds per grain.",durhi,rate);     
		return(DONT_SCHEDULE);
	}
	durti  = (double)p[12];
	loclo  = (double)p[13]; 
	locmid  = (double)p[14]; if (locmid < loclo ) locmid = loclo;
	lochi  = (double)p[15]; if (lochi < locmid ) lochi = locmid;
	locti  = (double)p[16];
	transplo  = (double)p[17];  
	transpmid  = (double)p[18]; 
        if (transpmid < transplo ) transpmid = transplo;
	transphi  = (double)p[19];  
        if (transphi < transpmid ) transphi = transpmid;
	transpti  = (double)p[20];
	granlyrs = (int)p[21];
	srrand((int)(p[22]));

	return nSamps();
}

int STGRANR::run()
{
        long i,attacksamps,j,thechunksamp,waitsamps,left,ngrains,rsamps;
        float *outp;
	float loc, ampfac, interval;
        double    frac;
	const int frameCount = framesToRun();
        
	if (in == NULL)        /* first time, so allocate it */
		in = new float [RTBUFSAMPS * inputChannels()];

//        if ( (durhi*(float)SR) > frameCount())
//		rtcmix_advise("STGRANR", "Grain duration larger than buffer.");
        
    outp = outbuf;               /* point to inst private out buffer */

	// figure out how many grains are in this chunk 
	ngrains = (int)((float)frameCount/(rate*(float)SR));	
        
	if ( ngrains  < 1 ) {
                if ( grainoverlap ) 
                    ngrains = 1;
                else {
					if ( (rrand()*.5+.5) < ((float)frameCount/(rate*(float)SR)) )
                        ngrains = 1;
                    else 
                         ngrains = 0;
                }
        }
        
        thechunksamp = 0;
        if ( ngrains ) {  
             for (i = 0; i < ngrains; i++) {
				attacksamps = frameCount/ngrains; // no grainoverlap yet
                transp = (float)prob(transplo, transpmid, transphi, transpti);
                interval = octpch(transp);
                increment = (double) cpsoct(10.0 + interval) / cpsoct(10.0);
                gdur = (float)prob(durlo,durmid,durhi,durti);
                grainsamps = (long)(gdur*(float)SR);    
                if ( grainsamps > attacksamps) {
                    overlapsample = grainsamps - attacksamps; // where to start in next attacksamps in envelop
                    grainoverlap = 1;
                    grainsamps = attacksamps;
                }
		ratevar = (float)prob(ratevarlo, ratevarmid, ratevarhi, ratevarti);
                waitsamps = (long)(ratevar*(float)(attacksamps - grainsamps));
		spread = (float)prob(loclo,locmid,lochi,locti);
		tableset(SR, gdur, grlen, tabg); 
		for ( j = 0; j < attacksamps; j++ ) {
			if (--branch < 0) { 
				aamp = tablei(currentFrame(), amptable, tabs) * amp;
				branch = skip;
			}
                        while (get_frame) {
                            if (inframe >= attacksamps) {
                                rtgetin(in, this, attacksamps * inputChannels());
                                inframe = 0;
                            }
                            oldersig[0] = oldsig[0];
                            oldsig[0] = newsig[0];
                            newsig[0] = in[(inframe * inputChannels())/* + inchan*/];
                            if ( inputChannels() == 2 ) {
                                oldersig[1] = oldsig[1];
                                oldsig[1] = newsig[1];
                                newsig[1] = in[(inframe * inputChannels())+1];
                            }
                            
                            inframe++;
                            incount++;
                    
                            if (counter - (double) incount < 0.5)
                                get_frame = 0;
                        }
                        if(( j < grainsamps + waitsamps) && ( j > waitsamps )) { 
                            ampfac = tablei(j-waitsamps,grenvtable,tabg);
                            frac = (counter - (double) incount) + 2.0;
                            outp[0] = interp(oldersig[0], oldsig[0], newsig[0], frac) * ampfac;
                            if (inputChannels() == 2 && outputChannels() == 2)  // split stereo files between the channels 
                                outp[1] = interp(oldersig[1], oldsig[1], newsig[1], frac) * ampfac;
                        }
                        else 
                            outp[0] = 0.0;
                        if (outputChannels() == 2 ) {  // split stereo files between the channels 
                            outp[1] = (1.0 - spread) * outp[0];
                            outp[0] *= spread;
                        }
						Instrument::increment(); // sample of whole note
                        thechunksamp++; // sample within chunk
                        outp += outputChannels();  
                        counter += increment;         /* keeps track of interp pointer */
                        if (counter - (double) incount >= -0.5)
                            get_frame = 1;
		}
                totalgrains++;
            }
        }
        else { // ngrains = 0
			for ( j = 0; j < frameCount; j++ ) {
                outp[0] = 0.0;
                if (outputChannels() == 2) outp[1] = 0.0;
                outp += outputChannels();
				Instrument::increment(); // sample of whole note
                thechunksamp++; // sample within chunk
           }
        }
 //       rtcmix_advise("STGRANR", "totalgrains: %ld\n",totalgrains);
        return thechunksamp;        
}

double STGRANR::prob(double low,double mid,double high,double tight)  
        // Returns a value within a range close to a preferred value 
	
                    // tightness: 0 max away from mid
                     //               1 even distribution
                      //              2+amount closeness to mid
                      //              no negative allowed 
{
	int repeat;
	double range, num, sign;

	range = (high-mid) > (mid-low) ? high-mid : mid-low;
	do {
	  	if (rrand() > 0.)  
			sign = 1.; 
		else  sign = -1.;
	  	num = mid + sign*(pow((rrand()+1.)*.5,tight)*range);
	} while(num < low || num > high);
	return(num);
}


Instrument*
makeSTGRANR()
{
	STGRANR *inst;
	inst = new STGRANR();
        inst->set_bus_config("STGRANR");
	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("STGRANR",makeSTGRANR);
}
#endif
