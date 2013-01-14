#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "SGRANR.h"
#include <rt.h>
#include <rtdefs.h>
#include <math.h>

// realtime stochastic grain maker based on sgran
// Mara Helmuth March 19, 2004

SGRANR::SGRANR() : Instrument()
{
	branch = 0;
        grainoverlap = totalgrains = 0;
}

int SGRANR::init(double p[], int n_args)
{
/*  p0=start time
*  p1=duration
*  p2=amplitude
*  p3=rate, p4-7 ratevar
*  p8-11 duration
*  p12-15 location
*  p16-19 transposition
*  function slot 1 is amp envelop
*   slot 2 is waveform 
*   slot 3 is grain env
*/
        starttime = p[0];
        evdur = p[1];

	if (rtsetoutput(starttime, evdur, this) == -1)
		return(DONT_SCHEDULE);

	if (outputChannels() > 2) {
		die("SGRANR", "Can't handle more than 2 output channels.");
		return(DONT_SCHEDULE);
	}

	wavetable = floc(2);
	if (wavetable == NULL) {
		die("SGRANR", "You need to store a waveform in function %d.", 1);
		return(DONT_SCHEDULE);
	}
	len = fsize(2);

	phase = 0.0;
	amp = p[2];
	
	amptable = floc(1);
	if (amptable) {
            alen = fsize(1);
            tableset(SR, evdur, alen, tabs);
	}
	else
		rtcmix_advise("SGRANR", "Setting phrase curve to all 1's.");
	grenvtable = floc(3);
	if (grenvtable) {
            grlen = fsize(3);
	}
	else
		rtcmix_advise("SGRANR", "Setting grain envelope to all 1's.");
        
	aamp = amp;
        
	skip = (int)(SR/(float)resetval);

	rate = p[3]; 
	ratevarlo  = (double)p[4];
	ratevarmid  = (double)p[5]; if (ratevarmid < ratevarlo ) ratevarmid = ratevarlo;
	ratevarhi  = (double)p[6]; if (ratevarhi < ratevarmid ) ratevarhi = ratevarmid;
	ratevarti  = (double)p[7];
	durlo  = (double)p[8];
	durmid  = (double)p[9]; if (durmid < durlo ) durmid = durlo;
	durhi  = (double)p[10]; if (durhi < durmid ) durhi = durmid;
   if ( durhi > rate ) {
 		die("SGRANR", "Grain durations %f sec. are larger than rate: %f seconds per grain.",durhi,rate);
		return(DONT_SCHEDULE);
	}
	durti  = (double)p[11];
	loclo  = (double)p[12]; 
	locmid  = (double)p[13]; if (locmid < loclo ) locmid = loclo;
	lochi  = (double)p[14]; if (lochi < locmid ) lochi = locmid;
	locti  = (double)p[15];
	freqlo  = (double)p[16];  if (freqlo < 15.0 ) freqlo = cpspch(freqlo);
	freqmid  = (double)p[17]; if (freqmid < 15.0 ) freqmid = cpspch(freqmid); 
        if (freqmid < freqlo ) freqmid = freqlo;
	freqhi  = (double)p[18];  if (freqhi < 15.0 ) freqhi = cpspch(freqhi); 
        if (freqhi < freqmid ) freqhi = freqmid;
	freqti  = (double)p[19];
	granlyrs = (int)p[20];
	srrand((int)(p[21]));

	return nSamps();
}

int SGRANR::run()
{
   long i,attacksamps,j,thechunksamp,waitsamps,left,ngrains;
	float out[2];
	float loc;
    const int frameCount = framesToRun();
	
//        if ( (durhi*(float)SR) > frameCount)
//		rtcmix_advise("SGRANR", "Grain duration larger than buffer.");

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
                    // first, finish writing any grains from previous frame block or attacksamps
                if ( grainoverlap ) {
                    left = ( grainsamps - overlapsample );
                    if ( left < 0 ) 
                        left = 0;
                    for ( j = 0; j < left; j++ ) {
                        if (--branch < 0) { 
                            aamp = tablei(currentFrame(), amptable, tabs) * amp;
                            branch = skip;
                        }
                        out[0] = oscili(aamp, si, wavetable, len, &phase);
                        out[0] = out[0] * tablei(overlapsample++,grenvtable,tabg);
                        if (outputChannels() == 2) { // split stereo files between the channel!s 
                            out[1] = (1.0 - spread) * out[0];
                            out[0] *= spread;
                        }
                        rtaddout(out);
                        increment(); // sample of whole note
                        totalgrains++;
                    }
                    grainoverlap = 0;
                }
                else
                    left = 0;
                
				attacksamps = (frameCount - left)/ngrains; // samples betwn attacks
                left = 0;
                freq = (float)prob(freqlo, freqmid, freqhi, freqti);
                si = freq * (float)len/SR;
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
                        if(( j < grainsamps + waitsamps) && ( j > waitsamps )) { 
                            out[0] = oscili(aamp, si, wavetable, len, &phase);
                            out[0] = out[0] * tablei(j-waitsamps,grenvtable,tabg);
                        }
                        else 
                            out[0] = 0.0;
                        if (outputChannels() == 2) { // split stereo files between the channels 
                            out[1] = (1.0 - spread) * out[0];
                            out[0] *= spread;
                        }
                        rtaddout(out);
						increment(); // sample of whole note
                        thechunksamp++; // sample within chunk
		}
                totalgrains++;
            }
        }
        else { // ngrains = 0
			for ( j = 0; j < frameCount; j++ ) {
                out[0] = 0.0;
                if (outputChannels() == 2) out[1] = 0.0;
                rtaddout(out);
                increment(); // sample of whole note
                thechunksamp++; // sample within chunk
           }
        }
 //       rtcmix_advise("SGRANR", "totalgrains: %ld\n",totalgrains);
        return thechunksamp;        
}

double SGRANR::prob(double low,double mid,double high,double tight)  
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
makeSGRANR()
{
	SGRANR *inst;
	inst = new SGRANR();
        inst->set_bus_config("SGRANR");
	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("SGRANR",makeSGRANR);
}
#endif
