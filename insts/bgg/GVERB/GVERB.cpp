/* GVERB -- long reverberator

	based on the original "gverb/gigaverb" by Juhana Sadeharju
		kouhia at nic.funet.fi
	code taken from the Max/MSP port by Olaf Matthes, <olaf.matthes@gmx.de>
	(see "origcopyright.txt" accompanying this instrument)

   p0 = output start time
   p1 = input start time
   p2 = duration
   *p3 = output amp multiplier
   *p4 = roomsize (1.0 -- 300.0)
   *p5 = reverb time (0.1 -- 360.0)
   *p6 = damping (0.0 -- 1.0)
   *p7 = input bandwidth (0.0 -- 1.0)
   *p8 = dry level (inverse dB, -90.0 -- 0.0)
   *p9 = early reflection level (inverse dB, -90.0 -- 0.0)
   *p10 = tail level (inverse dB, -90.0 -- 0.0)
	p11 = ring-down time (added to duration)
	p12 = input channel [optional, default = 0]

   * p-fields marked with an asterisk can receive dynamic updates
   from a table or real-time control source

	pfield tables will extend over the duration + ring-down time

   BGG, 5/2010
*/


#include <stdlib.h>
#include <Instrument.h>
#include <ugens.h>
#include <Ougens.h>
#include "GVERB.h"
#include <rt.h>
#include <rtdefs.h>


GVERB::GVERB() : Instrument()
{
}

GVERB::~GVERB()
{
}

int GVERB::init(double pfs[], int n_args)
{
	if (rtsetoutput(pfs[0], pfs[2]+pfs[11], this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() != 2)
		return die("GVERB", "stereo output required");
	if (rtsetinput(pfs[1], this) == -1)
		return DONT_SCHEDULE;   // no input

	inputframes = pfs[2] * SR;
	inputchan = pfs[12];

	amp = pfs[3];

	float maxroomsize = 300.0f;
	float roomsize = 50.0f;
	float revtime = 7.0f;
	float damping = 0.5f;
	float spread = 15.0f;
	float inputbandwidth = 0.5f;
	float drylevel = 0.0f; //-1.9832f;
	float earlylevel = 0.0f; //-1.9832f;
	float taillevel = 0.0f;

	float ga,gb,gt;
	unsigned int i;
	int n;
	float r;
	float diffscale;
	int a,b,c,cc,d,dd,e;
	float spread1,spread2;

	// BGG max/msp heritage, params/etc. stored in this "p" struct (ty_gverb)
	p = &realp;
	// zero out the struct, to be careful
	bzero((void *)p, sizeof (ty_gverb));

	p->rate = SR;
	p->fdndamping = damping;
	p->maxroomsize = maxroomsize;
	p->roomsize = CLIP(roomsize, 0.1f, maxroomsize);
	p->revtime = revtime;
	p->drylevel = drylevel;
	p->earlylevel = earlylevel;
	p->taillevel = taillevel;

	p->maxdelay = p->rate*p->maxroomsize/340.0;
	p->largestdelay = p->rate*p->roomsize/340.0;

	/* Input damper */

	p->inputbandwidth = inputbandwidth;
	p->inputdamper = damper_make(1.0 - p->inputbandwidth);


	/* FDN section */

	p->fdndels = (ty_fixeddelay **)malloc(FDNORDER*sizeof(ty_fixeddelay *));
	if(!p->fdndels)
		return die("GVERB", "out of memory for fixeddelay ptrs");
	for(i = 0; i < FDNORDER; i++)
	{
		p->fdndels[i] = fixeddelay_make((int)p->maxdelay+1000);
		if(!p->fdndels[i])
			return die("GVERB", "out of memory for fixeddelays");
	}
	p->fdngains = (float *)malloc(FDNORDER*sizeof(float));
	p->fdnlens = (int *)malloc(FDNORDER*sizeof(int));
	if(!p->fdngains || !p->fdnlens)
		return die("GVERB", "out of memory for delay gains and lengths");

	p->fdndamps = (ty_damper **)malloc(FDNORDER*sizeof(ty_damper *));
	if(!p->fdndamps)
		return die("GVERB", "out of memory for delay amps");

	for(i = 0; i < FDNORDER; i++)
	{
		p->fdndamps[i] = damper_make(p->fdndamping);
		if(!p->fdndamps[i])
			return die("GVERB", "out of memory for delay amps 2");
	}

	ga = 60.0;
	gt = p->revtime;
	ga = pow(10.0,-ga/20.0);
	n = (int)(p->rate*gt);
	p->alpha = pow((double)ga,(double)1.0/(double)n);

	gb = 0.0;
	for(i = 0; i < FDNORDER; i++)
	{
		if (i == 0) gb = 1.000000*p->largestdelay;
		if (i == 1) gb = 0.816490*p->largestdelay;
		if (i == 2) gb = 0.707100*p->largestdelay;
		if (i == 3) gb = 0.632450*p->largestdelay;

#if 0
		p->fdnlens[i] = nearest_prime((int)gb, 0.5);
#else
		p->fdnlens[i] = (int)gb;
#endif
		// p->fdngains[i] = -pow(p->alpha,(double)p->fdnlens[i]);
		p->fdngains[i] = -powf((float)p->alpha,p->fdnlens[i]);
	}

	p->d = (float *)malloc(FDNORDER*sizeof(float));
	p->u = (float *)malloc(FDNORDER*sizeof(float));
	p->f = (float *)malloc(FDNORDER*sizeof(float));
	if(!p->d || !p->u || !p->f)
		return die("GVERB", "out of memory for other delay stuff");


	/* Diffuser section */

	diffscale = (float)p->fdnlens[3]/(210+159+562+410);
	spread1 = spread;
	spread2 = 3.0*spread;

	b = 210;
	r = 0.125541f;
	a = (int)(spread1*r);
	c = 210+159+a;
	cc = c-b;
	r = 0.854046f;
	a = (int)(spread2*r);
	d = 210+159+562+a;
	dd = d-c;
	e = 1341-d;

	p->ldifs = (ty_diffuser **)malloc(4*sizeof(ty_diffuser *));
	if(!p->ldifs)
		return die("GVERB", "out of memory for diffuser left structs");

	p->ldifs[0] = diffuser_make((int)(diffscale*b),0.75);
	p->ldifs[1] = diffuser_make((int)(diffscale*cc),0.75);
	p->ldifs[2] = diffuser_make((int)(diffscale*dd),0.625);
	p->ldifs[3] = diffuser_make((int)(diffscale*e),0.625);
	if(!p->ldifs[0] || !p->ldifs[1] || !p->ldifs[2] || !p->ldifs[3])
		return die("GVERB", "out of memory for diffuser left makes");

	b = 210;
	r = -0.568366f;
	a = (int)(spread1*r);
	c = 210+159+a;
	cc = c-b;
	r = -0.126815f;
	a = (int)(spread2*r);
	d = 210+159+562+a;
	dd = d-c;
	e = 1341-d;

	p->rdifs = (ty_diffuser **)malloc(4*sizeof(ty_diffuser *));
	if(!p->rdifs)
		return die("GVERB", "out of memory for diffuser right structs");

	p->rdifs[0] = diffuser_make((int)(diffscale*b),0.75);
	p->rdifs[1] = diffuser_make((int)(diffscale*cc),0.75);
	p->rdifs[2] = diffuser_make((int)(diffscale*dd),0.625);
	p->rdifs[3] = diffuser_make((int)(diffscale*e),0.625);
	if(!p->rdifs[0] || !p->rdifs[1] || !p->rdifs[2] || !p->rdifs[3])
		return die("GVERB", "out of memory for diffuser right makes");

	/* Tapped delay section */

	p->tapdelay = fixeddelay_make(44000);
	p->taps = (int *)malloc(FDNORDER*sizeof(int));
	p->tapgains = (float *)malloc(FDNORDER*sizeof(float));
	if(!p->tapdelay || !p->taps || !p->tapgains)
		return die("GVERB", "out of memory for taps");

	p->taps[0] = (int)(5+0.410*p->largestdelay);
	p->taps[1] = (int)(5+0.300*p->largestdelay);
	p->taps[2] = (int)(5+0.155*p->largestdelay);
	p->taps[3] = (int)(5+0.000*p->largestdelay);

	for(i = 0; i < FDNORDER; i++)
	{
		p->tapgains[i] = pow(p->alpha,(double)p->taps[i]);
	}


	// these values get set after all the init stuff
	if (pfs[4] < 1.0 || pfs[4] > maxroomsize) 
		return die("GVERB", "bogus roomsize: %f\n", pfs[4]);
	gverb_set_roomsize(p, pfs[4]); // sets p->roomsize

	if (pfs[5] < 0.1 || pfs[5] > 360.0)
		return die("GVERB", "bad revtime: %f\n", pfs[5]);
	gverb_set_revtime(p, pfs[5]);

	if (pfs[6] < 0.0 || pfs[6] > 1.0)
		return die("GVERB", "incorrect damping: %f\n", pfs[6]);
	gverb_set_damping(p, pfs[6]);

	if (pfs[7] < 0.0 || pfs[7] > 1.0)
		return die("GVERB", "input bandwith problem: %f\n", pfs[7]);
	gverb_set_inputbandwidth(p, pfs[7]);

	if (pfs[8] < -90.0 || pfs[8] > 0.0)
		return die("GVERB", "dry level wrong: %f\n", pfs[8]);
	gverb_set_drylevel(p, pfs[8]);

	if (pfs[9] < -90.0 || pfs[9] > 0.0)
		return die("GVERB", "problem with early reflection level: %f\n", pfs[9]);
	gverb_set_earlylevel(p, pfs[9]);

	if (pfs[10] < -90.0 || pfs[10] > 0.0)
		return die("GVERB", "bogus tail level: %f\n", pfs[10]);
	gverb_set_taillevel(p, pfs[10]);

	branch = 0;

	return nSamps();
}

int GVERB::configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


void GVERB::doupdate()
{
	double pfs[11];

	update(pfs, 11);

	amp = pfs[3];

	if (pfs[4] != p->roomsize) {
		if (pfs[4] < 1.0 || pfs[4] > p->maxroomsize) 
			rtcmix_warn("GVERB", "bogus roomsize: %f\n", pfs[4]);
		gverb_set_roomsize(p, pfs[4]); // sets p->roomsize
	}

	if (pfs[5] != p->revtime) {
		if (pfs[5] < 0.1 || pfs[5] > 360.0)
			rtcmix_warn("GVERB", "bad revtime: %f\n", pfs[5]);
		gverb_set_revtime(p, pfs[5]);
	}

	if (pfs[6] != p->fdndamping) {
		if (pfs[6] < 0.0 || pfs[6] > 1.0)
			rtcmix_warn("GVERB", "incorrect damping: %f\n", pfs[6]);
		gverb_set_damping(p, pfs[6]);
	}

	if (pfs[7] != p->inputbandwidth) {
		if (pfs[7] < 0.0 || pfs[7] > 1.0)
			rtcmix_warn("GVERB", "input bandwith problem: %f\n", pfs[7]);
		gverb_set_inputbandwidth(p, pfs[7]);
	}

	if (DB_CO(pfs[8]) != p->drylevel) {
		if (pfs[8] < -90.0 || pfs[8] > 0.0)
			rtcmix_warn("GVERB", "dry level wrong: %f\n", pfs[8]);
		gverb_set_drylevel(p, pfs[8]);
	}

	if (DB_CO(pfs[9]) != p->earlylevel) {
		if (pfs[9] < -90.0 || pfs[9] > 0.0)
			rtcmix_warn("GVERB", "problem with early reflection level: %f\n", pfs[9]);
			gverb_set_earlylevel(p, pfs[9]);
	}

	if (DB_CO(pfs[10]) != p->taillevel) {
		if (pfs[10] < -90.0 || pfs[10] > 0.0)
			rtcmix_warn("GVERB", "bogus tail level: %f\n", pfs[10]);
		gverb_set_taillevel(p, pfs[10]);
	}
}

int GVERB::run()
{
	const int samps = framesToRun() * inputChannels();
	int i;
	float out[2];

	rtgetin(in, this, samps);
	
	for (i = 0; i < samps; i += inputChannels()) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}

		if (currentFrame() > inputframes) in[i+inputchan] = 0.0;
		gverb_do(p, in[i+inputchan], out, out+1);

		out[0] = (out[0] * amp) + (in[i+inputchan] * p->drylevel);
		out[1] = (out[1] * amp) + (in[i+inputchan] * p->drylevel);

		rtaddout(out);

		increment();
	}
	return i;
}

Instrument*
makeGVERB()
{
	GVERB *inst;
	inst = new GVERB();
	inst->set_bus_config("GVERB");
	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("GVERB",makeGVERB);
}
#endif
