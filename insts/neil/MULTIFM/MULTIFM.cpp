/* MULTIFM - configurable multi-oscillator FM synthesis instrument

   p0 = output start time
   p1 = duration
   p2 = overall amplitude multiplier
   p3 = number of oscillators
   p4 = pan (in percent-to-left format)
   p5, p6 ... pN-1, pN
     List of frequency / waveform pairs for each oscillator.
   pO, pO+1, pO+2 ... pP-2, pP-1, pP
     Oscillator connections in triples in the following form:

     1) Oscillator (Carrier) - Audio Out - Relative Amplitude
     or
     2) Modulator - Carrier - Index

     The first number refers to an oscillator, 1-N.
       1 refers to the first frequency / waveform pair, p5-p6.
       2 refers to the next pair, and so on.

     The second number directs the output of the indicated oscillator.
       0 directs it to audio output (form 1 above).
       1-N directs it to modulate the frequency of the given
         oscillator (form 2 above).

     The third number is the relative amplitude (for form 1) or the
     index (for form 2).

   Any number of connections in any direction, including feedback,
   is acceptable.

   p2 amplitude, p4 pan, oscillator frequencies, and oscillator
   amplitude/index can receive dynamic updates from a table or
   real-time control source.

   Neil Thornock <neilthornock at gmail>, 11/2016.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include "MULTIFM.h"
#include <rt.h>
#include <rtdefs.h>

#include <iostream>

MULTIFM::MULTIFM()
	: branch(0)
{
	numops = 0;
	oscil = NULL;
	nummodspercar = NULL;
	allmodsperop = NULL;
	allampsperop = NULL;
	allampsperopind = NULL;
	phs = NULL;
	phsinc = NULL;
	wavelentab = NULL;
	outops = NULL;
	opsnotout = NULL;
	outamps = NULL;
	outampsind = NULL;
	sinetable = NULL;
}

MULTIFM::~MULTIFM()
{
	for (int i = 0; i < numops; i++) {
		delete oscil[i];
		delete allmodsperop[i];
		delete allampsperop[i];
		delete allampsperopind[i];
	}

	delete [] oscil;
	delete [] nummodspercar;
	delete [] allmodsperop;
	delete [] allampsperop;
	delete [] allampsperopind;
	delete [] phs;
	delete [] phsinc;
	delete [] wavelentab;
	delete [] outops;
	delete [] opsnotout;
	delete [] outamps;
	delete [] outampsind;
	delete [] sinetable;
}

double MULTIFM::fmodpos(double num, int length)
{
	// modulo, but only returns positive values
	if (num >= 0)
		return fmod(num, length);
	else {
		while (num < 0)
			num += length;
		return num;
	}
}

void MULTIFM::initarrays(int numops)
{
	oscil = new Ooscili * [numops];
	phs = new double [numops];
	phsinc = new double [numops];
	wavelentab = new double [numops];
	outops = new int [numops];
	outamps = new double [numops];
	outampsind = new int [numops];
	opsnotout = new int [numops];
	allmodsperop = new int * [numops];
	allampsperop = new double * [numops];
	allampsperopind = new int * [numops];

	for (int i = 0; i < numops; i++) {
		allmodsperop[i] = new int[0];
		allampsperop[i] = new double[0];
		allampsperopind[i] = new int[0];
	}
}

void MULTIFM::createarrays(double p[])
{
	int firstmod = 5 + numops * 2;
	numouts = 0;
	nummodspercar = new int [numops];
	for (int i = 0; i < numops; i++) {
		nummodspercar[i] = 0;
	}
	while (firstmod < nargs) {
		int i = firstmod + 1;
		int thiscar = p[i] - 1;
		if (p[firstmod + 1] == 0) {
			outops[numouts] = p[firstmod] - 1;
			outamps[numouts] = p[firstmod + 2];
			outampsind[numouts] = firstmod + 2;
			numouts++;
		}
		else if (nummodspercar[thiscar] == 0) {
			nummodspercar[thiscar] = 1;
			for (int j = i + 3; j < nargs; j += 3) {
				if (p[i] == p[j]) {
					nummodspercar[thiscar] += 1;
				}
			}
			int nummodsthiscar = nummodspercar[thiscar];
			delete allmodsperop[thiscar];
			delete allampsperop[thiscar];
			delete allampsperopind[thiscar];
			allmodsperop[thiscar] = new int[nummodsthiscar];
			allampsperop[thiscar] = new double[nummodsthiscar];
			allampsperopind[thiscar] = new int[nummodsthiscar];
			allmodsperop[thiscar][0] = p[firstmod] - 1;
			allampsperop[thiscar][0] = p[firstmod + 2];
			allampsperopind[thiscar][0] = firstmod + 2;
			if (nummodsthiscar > 1) {
				int k = 1;
				for (int j = i + 3; j < nargs; j += 3) {
					if (p[i] == p[j]) {
						allmodsperop[thiscar][k] = p[j - 1] - 1;
						allampsperop[thiscar][k] = p[j + 1];
						allampsperopind[thiscar][k] = j + 1;
						k += 1;
					}
				}
			}
		}
		firstmod += 3;
	}
}

int MULTIFM::init(double p[], int n_args)
{
	double *pfields = p;
	nargs = n_args;
	const float outskip = p[0];
	const float dur = p[1];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() < 1 || outputChannels() > 2)
		return die("MULTIFM", "Use mono or stereo output only.");

	numops = p[3];
	initarrays(numops);

	for (int i = 0; i < numops; i++) {
		double *wavet;
		int this_freq = 5 + (i * 2);
		wavet = (double *) getPFieldTable(this_freq + 1, &wavelen);
		wavelentab[i] = wavelen;
		if (wavet == NULL)
			return die("MULTIFM", "must provide wavetables for each oscillator");
		oscil[i] = new Ooscili(SR, 440.0, wavet, wavelen);
		phsinc[i] = p[this_freq] / SR;
		phs[i] = 0;
	}

	createarrays(pfields);

	return nSamps();
}

int MULTIFM::configure()
{
	return 0;
}

void MULTIFM::doupdate()
{
	// BGGx ww arg!
	//double p[nargs];
	double *p = new double[nargs];
	update(p, nargs);

	for (int i = 0; i < numops; i++) {
		double thisfreq = p[5 + (2 * i)];
		oscil[i]->setfreq(thisfreq);
		phsinc[i] = thisfreq / SR;
	}

	double scaleamp = 0;
	for (int i = 0; i < numouts; i++) {
		int amppfield = outampsind[i];
		outamps[i] = p[amppfield];
		scaleamp += outamps[i];
	}
	for (int i = 0; i < numouts; i++) {
		outamps[i] = outamps[i] / scaleamp;
	}

	for (int i = 0; i < numops; i++) {
		for (int j = 0; j < nummodspercar[i]; j++) {
			int amppfield = allampsperopind[i][j];
			allampsperop[i][j] = p[amppfield];
		}
	}

	amp = p[2];
	pan = p[4];
}

int MULTIFM::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}

		for (int i = 0; i < numops; i++) {
			double index = 0;
			double modamt = 0;
			for (int j = nummodspercar[i]-1; j >= 0; j--) {
				int thismodind = allmodsperop[i][j];
				index = allampsperop[i][j] / (M_PI * 2); // radians
				modamt = modamt + oscil[thismodind]->next() * index;
			}
			double carphase = fmodpos((phs[i] + modamt) * wavelentab[i], wavelentab[i]);
			oscil[i]->setphase(carphase);
			// increment the phase of the carrier
			phs[i] = fmod(phs[i] + phsinc[i], 1);
		}

		float thisout = 0;
		for (int i = 0; i < numouts; i++) {
			int outopind = outops[i];
			thisout += oscil[outopind]->next() * outamps[i];
		}
		thisout = thisout * ( 1.0 / float(numouts)) * amp;

		float out[2];
		if (outputChannels() == 1) {
			out[0] = thisout;
		}
		else {
			out[0] = thisout * pan;
			out[1] = thisout * (1.0 - pan);
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}

Instrument *makeMULTIFM()
{
	MULTIFM *inst = new MULTIFM();
	inst->set_bus_config("MULTIFM");

	return inst;
}

#ifndef EMBEDDED
void rtprofile()
{
	RT_INTRO("MULTIFM", makeMULTIFM);
}
#endif
