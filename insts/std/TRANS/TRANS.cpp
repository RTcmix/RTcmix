#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../sys/mixerr.h"
#include "TRANS.h"
#include "../../rtstuff/rt.h"

#define DEBUG

extern "C" {
#include "../../H/ugens.h"
}

float interp(float, float, float, float);

extern int resetval;
extern int lineset;

TRANS::TRANS() : Instrument()
{
	incount = 2.0f;
	counter = 0;
}

//  p0 = outskp  p1 = inskp  p2 = dur (tmnd)  p3 = intrvl of trnsp. p4 = amp
//  we're stashing the setline info in gen table 1

int
TRANS::init(float p[], short n_args)
{
	if (n_args != 5) {
             cerr << "Wrong number of args for TRANS!" << endl;
             return -1;
	}
	if (p[2] < 0.0) p[2] = p[2] - p[1];
	nsamps = rtsetoutput(p[0], p[2], this);
	rtsetinput(p[1], this);
        float interval = octpch(p[3]);  /* convert interval to lin octave */
        increment = cpsoct(10.0+interval)/cpsoct(10.0);  /* the samp incr. */
	amp = p[4];
	
	printf("increment: %g\n", increment);
	
	if (lineset) {
		amptable = floc(1);
		int amplen = fsize(1);
		tableset(p[2], amplen, tabs);
	}
	
	skip = SR/(float)resetval; // how often to update amp curve, default 200/sec.
	
	// clear sample history
	
        for(int n=0; n < NCHANS; n++) {
           vold[n] = 0;
           old[n] = 0;
        }

	return(nsamps);
}

inline int min(int x, int y) { if (x < y) return x; else return y; }

int
TRANS::run()
{
	const int nchans = NCHANS;
        const int sampsout = 0, outsamps = chunksamps;
	int insamp = 0, sampsread = 0;
	float in[MAXCHANS*(MAXBUF+2)], out[MAXCHANS*MAXBUF], aamp;
	int branch = 0;
	float *inp = &in[0], *outp = &out[0];
	
	// read initial block of input at offset +2 frames
	
	int totalinleft = 0.5 + (outsamps * increment);
	int toRead = min(totalinleft, MAXBUF);
	printf("READ %d samples\n", toRead);
	rtgetin(&in[2 * nchans], this, toRead * inputchans);
	sampsread += toRead;

	// retrieve history and store into first two frames of input array
	
	for (int n = 0; n < nchans; n++) {
	    in[n] = vold[n];
	    in[n + nchans] = old[n];
	}

	for (int i = 0; i < outsamps; i++) {
	    if (insamp > sampsread) {
	        totalinleft = 0.5 + ((outsamps - i) * increment);
	        toRead = min(totalinleft, MAXBUF);
		printf("READ %d samples\n", toRead);
	        rtgetin(&in[0], this, toRead * inputchans);
		inp = &in[0];	// reset
		sampsread += toRead;
	    }
	    if (--branch < 0) {
		if (lineset)
		    aamp = table(cursamp, amptable, tabs) * amp;
		else
		    aamp = amp;
		branch = skip;
	    }
	    float frac = (counter - incount) + 2.0f;
#ifdef DEBUG
	    printf("i: %d counter: %g incount: %d frac: %g insamp: %d cursamp: %d\n",
	           i, counter, incount, frac, insamp, cursamp);
#endif
            for (n = 0; n < nchans; n++) {
#ifdef DEBUG
		printf("interping %g, %g, %g\n", inp[0], inp[1], inp[2]);
#endif
                outp[n] = interp(inp[0], inp[1], inp[2], frac) * aamp;
	    }
	    outp += nchans;
	    cursamp++;
	    counter += increment;	// keeps track of interp pointer

	    // increment input
            while ((counter - (float)incount) >= -0.5f) {
		inp += nchans;
		incount++;
		insamp++;
	    }
	}
	// save history for next time
	printf("saving samps %g, %g\n", in[insamp+1], in[insamp+2]);
	for (n = 0; n < nchans; n++) {
	    vold[n] = in[(nchans * (insamp + 1)) + n];
	    old[n] = in[(nchans * (insamp + 2)) + n];
	}
	rtbaddout(out, outsamps * nchans);
	printf("OUT %d samples\n\n", i);
	return (i);
}

Instrument *
makeTRANS()
{
	return new TRANS();
}

void
rtprofile()
{
	RT_INTRO("TRANS",makeTRANS);
}

static float
interp(float y0, float y1, float y2, float t)
{
    float hy2, hy0, a, b, c;
    
    a = y0;
    hy0 = y0/2.0f;
    hy2 =  y2/2.0f;
    b = (-3.0f * hy0) + (2.0f * y1) - hy2;
    c = hy0 - y1 + hy2;

    return(a + b*t + c*t*t);
}
