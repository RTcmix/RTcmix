/* MODULAR - patchable synth

	p0 = output start time
	p1 = duration

	BGG, 5/2022
*/

// BGGx
#include <PField.h>
#include <RTcmix.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include "../../../src/rtcmix/tableutils.h"

#include <Instrument.h>
#include <ugens.h>
#include <Ougens.h>
#include "MODULAR.h"
#include <rt.h>
#include <rtdefs.h>

MODULAR::MODULAR() : Instrument()
{
}

MODULAR::~MODULAR()
{
}

int MODULAR::init(double p[], int n_args)
{
	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() > 2)
		return die("MODULAR", "Can't handle more than 2 output channels.");

	outsig = new msigout; // not using an array, only one
	for (int i = 0; i < NSLOTS; i++) {
		oscils[i] = new moscil;
		envs[i] = new menv;
		moogvcfs[i] = new mmoogvcf;
	}

	totbytes = 0;
	struct stat file_stat;
	stat("TTT.txt", &file_stat);
	oldtime = file_stat.st_mtime;

	branch = 0;

	return nSamps();
}

extern char specarray[50];

void MODULAR::doupdate()
{
   struct stat file_stat;
   stat("TTT.txt", &file_stat);
	char linebuf[80];
	char *tok; // for the tokenizing
	char ttok[10]; // for the sscanf apparently needed for the ending of lines
						// (arg! strcmp()!)
// MAXTOKS and MAXLABEL defined in MODULAR.h at the top
	char toks[MAXTOKS][MAXLABEL];
	float fval1, fval2;
	int ival1, ival2;

/*
	LEGEND
	+ oscil osc# sigout sigout# sig
	+ oscil osc# sigout sigout# amp
	+ oscil osc# oscil(2) osc# freq
	+ oscil osc# oscil(2) osc# amp
	+ oscil osc# moogvcf moog# sig
	+ oscil osc# moogcvf moog# amp
	+ oscil osc# moogcvf moog# cfreq
	+ oscil osc# moogcvf moog# reson
	+ env env# sigout sigout# sig
	+ env env# sigout sigout# amp
	+ env env# osc osc# amp
	+ env env# osc osc# freq
	+ env env# moogvcf moog# amp
	+ env env# moogvcf moog# cfreq
	+ env env# moogvcf moog# reson
	+ moogvcf moog# sigout sigout# sig
	+ moogvcf moog# moogvcf(2) moog# sig
	= sigout sigout# amp ampval#
	= oscil osc# freq freqval#
	= oscil osc# amp ampval#
	= oscil osc# wave wave#
		wave# 0 == sine
		wave# 1 == saw
		wave# 2 == square
		wave# 3 == triangle
		wave# 4 == pulse
	= env env# ping #pingval
	= env env# time #timeval
	= env env# loop #loopflag
	= moogvcf moog# cfreq cfreqval#
	= moogvcf moog# reson resonval#
	= moogvcf moog# amp ampval#
	- oscil osc# sigout sigout# sig
	- oscil osc# sigout sigout# amp
	- oscil(2) osc# oscil osc# amp
	- oscil(2) osc# oscil osc# freq
	- oscil osc# moogvcf moog# sig
	- oscil osc# moogvcf moog# amp
	- oscil osc# moogvcf moog# cfreq
	- oscil osc# moogvcf moog# reson
	- env env# sigout sigout# sig
	- env env# sigout sigout# amp
	- env env# oscil osc# amp
	- env env# oscil osc# freq
	- moogvcf moog# sigout sigout# sig
	- moogvcf moog# moogvcf(2) moog#
*/
	

	if ( (file_stat.st_mtime != oldtime) || (specarray[0] != 'X') ) {
		fd = open ("TTT.txt", O_RDONLY);
		lseek(fd, totbytes, SEEK_SET);
		nbytes = read(fd, (void *)linebuf, 80);
		totbytes += nbytes;

	// blitz out any leftover toks
	for (int i = 0; i < MAXTOKS; i++) *toks[i] = (char)NULL;

// BGGx  specarray for p interface (CMIX -o), linebuf for CMIX < MM.sco
		char *ptr = linebuf;
//		char *ptr = specarray;
		int numtoks = 0;
		while ((tok = strsep(&ptr, " ")) != NULL) {
			if (*tok != '\0') {
				strcpy(toks[numtoks++], tok);
			}
		}

		specarray[0] = 'X';  // now it's parsed, signal not to until new data

		// "+" -- we are connecting
		if (strcmp(toks[0], "+") == 0) {

			// oscil
			if (strcmp(toks[1], "oscil") == 0) {
				sscanf(toks[2], "%f", &fval1);
				ival1 = fval1;

				// connect oscil to sigout (note: not using ival2/fval2)
				if (strcmp(toks[3], "sigout") == 0)
					outsig->connect(oscils[ival1], toks[5]); // "amp"/"sig"

				// connect oscil to moogvcf
				if (strcmp(toks[3], "moogvcf") == 0) {
					sscanf(toks[4], "%f", &fval2);
					ival2 = fval2;
						// "sig"/"amp"/"reson"/"cfreq"
					moogvcfs[ival2]->connect(oscils[ival1], toks[5]);
				}

				// connect to another oscil
				if (strcmp(toks[3], "oscil") == 0) {
					sscanf(toks[4], "%f", &fval2);
					ival2 = fval2;
               oscils[ival2]->connect(oscils[ival1], toks[5]); // "freq"/"amp"
				}
			} // end of oscil connecting

			// moogvcf
			if (strcmp(toks[1], "moogvcf") == 0) {
				sscanf(toks[2], "%f", &fval1);
				ival1 = fval1;

            // connect moogvcf to sigout (note: not using ival2/fval2)
            if (strcmp(toks[3], "sigout") == 0)
               outsig->connect(moogvcfs[ival1], toks[5]); // "sig"

				if (strcmp(toks[3], "moogvcf") == 0) {	// cnx moogvcf to moogvcf
					sscanf(toks[4], "%f", &fval2);
					ival2 = fval2;
					moogvcfs[ival2]->connect(moogvcfs[ival1], toks[5]); // "sig"
				}
			} // end of moogvcf connecting


			// env
			if (strcmp(toks[1], "env") == 0) {
				sscanf(toks[2], "%f", &fval1);
				ival1 = fval1;

            // connect env to sigout (note: not using ival2/fval2)
            if (strcmp(toks[3], "sigout") == 0)
               outsig->connect(envs[ival1], toks[5]); // "amp"/"sig"

				// connect to oscil
				if (strcmp(toks[3], "oscil") == 0) {
					sscanf(toks[4], "%f", &fval2);
					ival2 = fval2;
               oscils[ival2]->connect(envs[ival1], toks[5]); // "freq"/"amp"
				}

				// connect to moogvcf
				if (strcmp(toks[3], "moogvcf") == 0) {
					sscanf(toks[4], "%f", &fval2);
					ival2 = fval2;
						// "sig"/"amp"/"reson"/"cfreq"
					moogvcfs[ival2]->connect(envs[ival1], toks[5]);
				}
			} // end of env connecting
		} // end of "+"


		// "=" -- we are setting a val
		if (strcmp(toks[0], "=") == 0) {

			// sigout
			if (strcmp(toks[1], "sigout") == 0) {
				sscanf(toks[2], "%f", &fval1);
				ival1 = fval1; // we don't use this for outsig

				if (strcmp(toks[3], "amp") == 0) { // set amp value
					sscanf(toks[4], "%f", &fval2);
					outsig->setamp(fval2);
				}
			} // end of sigout setting

			// oscil
			if (strcmp(toks[1], "oscil") == 0) {
				sscanf(toks[2], "%f", &fval1);
				ival1 = fval1;

				if (strcmp(toks[3], "freq") == 0) {
					sscanf(toks[4], "%f", &fval2);
					oscils[ival1]->setfreq(fval2);
				}
				if (strcmp(toks[3], "amp") == 0) {
					sscanf(toks[4], "%f", &fval2);
					oscils[ival1]->setamp(fval2);
				}
				if (strcmp(toks[3], "wave") == 0) {
					sscanf(toks[4], "%f", &fval2);
					oscils[ival1]->setwave((int)fval2);
				}
			} // end of oscil setting

			// env
			if (strcmp(toks[1], "env") == 0) {
				sscanf(toks[2], "%f", &fval1);
				ival1 = fval1;

				if (strcmp(toks[3], "ping") == 0) {
					sscanf(toks[4], "%f", &fval2);
					envs[ival1]->ping(fval2);
				}
				if (strcmp(toks[3], "time") == 0) {
               sscanf(toks[4], "%f", &fval2);
               envs[ival1]->settimelength(fval2);
            } 
				if (strcmp(toks[3], "loop") == 0) {
               sscanf(toks[4], "%f", &fval2);
               envs[ival1]->setloop((int)fval2);
            } 
			} // end of env setting

			// moogvcf
			if (strcmp(toks[1], "moogvcf") == 0) {
				sscanf(toks[2], "%f", &fval1);
				ival1 = fval1;
				if (strcmp(toks[3], "cfreq") == 0) {
					sscanf(toks[4], "%f", &fval2);
					moogvcfs[ival1]->setcfreq(fval2);
				}
				if (strcmp(toks[3], "reson") == 0) {
					sscanf(toks[4], "%f", &fval2);
					moogvcfs[ival1]->setres(fval2);
				}
				if (strcmp(toks[3], "amp") == 0) {
					sscanf(toks[4], "%f", &fval2);
					moogvcfs[ival1]->setamp(fval2);
				}
			} // end of moogvcf setting
		} // end of "="


		// "-" -- we are disconnecting
		if (strcmp(toks[0], "-") == 0) {

			// oscil
			if (strcmp(toks[1], "oscil") == 0) {
				sscanf(toks[2], "%f", &fval1);
				ival1 = fval1;

				// oscil connected to sigout  (note: not using ival2/fval2)
				if (strcmp(toks[3], "sigout") == 0)
					outsig->disconnect(oscils[ival1], toks[5]); // "amp"/"sig"

				// disconnect from another oscil
				if (strcmp(toks[3], "oscil") == 0) {
					sscanf(toks[4], "%f", &fval2);
					ival2 = fval2;
					oscils[ival2]->disconnect(oscils[ival1], toks[5]); //"freq"/"sig"
				}

				// disconnect from moogvcf
				if (strcmp(toks[3], "moogvcf") == 0) {
					sscanf(toks[4], "%f", &fval2);
					ival2 = fval2;
						// "sig"/"amp"/"reson"/"cfreq"
					moogvcfs[ival2]->disconnect(oscils[ival1], toks[5]);
				}
			} // end of oscil disconnecting

			// env
			if (strcmp(toks[1], "env") == 0) {
				sscanf(toks[2], "%f", &fval1);
				ival1 = fval1;

				// env connected to sigout  (note: not using ival2/fval2)
				if (strcmp(toks[3], "sigout") == 0)
					outsig->disconnect(envs[ival1], toks[5]); // "amp"/"sig"

				// env connected to oscil
				if (strcmp(toks[3], "oscil") == 0) {
					sscanf(toks[4], "%f", &fval2);
					ival2 = fval2;
					oscils[ival2]->disconnect(envs[ival1], toks[5]); // "freq"/"amp"
				}

				// env connected to moogvcf
				if (strcmp(toks[3], "moogvcf") == 0) {
					sscanf(toks[4], "%f", &fval2);
					ival2 = fval2;
						// "sig"/"amp"/"reson"/"cfreq"
					moogvcfs[ival2]->disconnect(envs[ival1], toks[5]);
				}
			} // end of env disconnecting

			// moogvcf
			if (strcmp(toks[1], "moogvcf") == 0) {
				sscanf(toks[2], "%f", &fval1);
				ival1 = fval1;

				// moogvcf connected to sigout
				if (strcmp(toks[3], "sigout") == 0)
					outsig->disconnect(moogvcfs[ival1], toks[5]); // "sig"

				// moogvcf connected to moogvcf
				if (strcmp(toks[3], "moogvcf") == 0) {
					sscanf(toks[4], "%f", &fval2);
					ival2 = fval2;
						// "sig"
					moogvcfs[ival2]->disconnect(moogvcfs[ival1], toks[5]); // sig
				}
			} // end of moogvcf disconnecting

		} // end of "-"

		oldtime = file_stat.st_mtime;
		close(fd);
	}
}

int MODULAR::run()
{
	int i,j;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
   	for (j = 0; j < NSLOTS; j++) {
	      oscils[j]->is_calculated = false;
	      envs[j]->is_calculated = false;
	      moogvcfs[j]->is_calculated = false;
	   }

		out[0] = out[1] = outsig->getval() * 10000.0;
		rtaddout(out);

		if (branch-- == 0) {
			doupdate();
			branch = getSkip();
		}

		increment();
	}
	return i;
}

Instrument*
makeMODULAR()
{
	MODULAR *inst;
	inst = new MODULAR();
	inst->set_bus_config("MODULAR");
	return inst;
}

#ifndef EMBEDDED
void
rtprofile()
{
	RT_INTRO("MODULAR",makeMODULAR);
}
#endif


// -------
moscil::moscil()
{
	// make a sine wave (the default)
	wavetable_from_string("sine", wavetable[0], TABLELEN, NULL);
	// saw
	wavetable_from_string("saw", wavetable[1], TABLELEN, NULL);
	// square
	wavetable_from_string("square", wavetable[2], TABLELEN, NULL);
	// triangle
	wavetable_from_string("tri", wavetable[3], TABLELEN, NULL);
	// buzz (pulse)
	wavetable_from_string("buzz", wavetable[4], TABLELEN, NULL);

	theoscil = new Ooscili(RTcmix::sr(), 350.0, wavetable[0], TABLELEN);

	amp = 1;

	freq = 350.0;
	for (int i = 0; i < NSLOTS; i++) freqarray[i] = NULL;
	for (int i = 0; i < NSLOTS; i++) amparray[i] = NULL;

	float retval;
}

float moscil::getval()
{
	if (!is_calculated) {
		float accum = 0.0;

		// calculate any freq control
		for (int i = 0; i < NSLOTS; i++) {
			if (freqarray[i] != NULL)
				accum += freqarray[i]->getval();
		}
		theoscil->setfreq(freq + accum);
	
		accum = 0.0;

		// calculate any amp control
		for (int i = 0; i < NSLOTS; i++) {
			if (amparray[i] != NULL)
				accum += amparray[i]->getval();
		}

		is_calculated = true;
		retval =  theoscil->next() * (amp + accum);
	}
	return retval;
}

void moscil::setfreq(float f)
{
	freq = f;
}

void moscil::setamp(float a)
{
	amp = a;
}

void moscil::setwave(int w)
{
	theoscil = new Ooscili(RTcmix::sr(), freq, wavetable[w], TABLELEN);
}

void moscil::connect(MODULES *m, char *in) {
	int i;
	char ttok[MAXLABEL];
	sscanf(in, "%s", ttok); // arg, strcmp!

	if (strcmp(ttok, "freq") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (freqarray[i] == NULL) {
				freqarray[i] = m;
				break;
			}
		}
	}

	if (strcmp(ttok, "amp") == 0) {
	   for (i = 0; i < NSLOTS; i++) {
  	   	if (amparray[i] == NULL) {
	         amparray[i] = m;
	         break;
  	    	}
  	 	}
	}
}

void moscil::disconnect(MODULES *m, char *in) {
	int i;
	char ttok[MAXLABEL];
	sscanf(in, "%s", ttok); // arg, strcmp!

	if (strcmp(ttok, "freq") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (freqarray[i] == m) {
				freqarray[i] = NULL;
				break;
			}
		}
	}

	if (strcmp(ttok, "amp") == 0) {
	   for (i = 0; i < NSLOTS; i++) {
  	   	if (amparray[i] == m) {
	         amparray[i] = NULL;
	         break;
  	    	}
  	 	}
	}
}


// -------
msigout::msigout()
{
	for (int i = 0; i < NSLOTS; i++) {
		sigarray[i] = NULL;
		amparray[i] = NULL;
	}

	amp = 1.0;
}

void msigout::setamp(float a)
{
	amp = a;
}

float msigout::getval()
{
	float accum = 0.0;

	for (int i = 0; i < NSLOTS; i++) {
		if (sigarray[i] != NULL)
			accum += sigarray[i]->getval();
	}

	float ampaccum = 0.0;

	// calculate any amp control
	for (int i = 0; i < NSLOTS; i++) {
		if (amparray[i] != NULL)
			ampaccum += amparray[i]->getval();
	}

	return accum * (amp + ampaccum);
}

void msigout::connect(MODULES *m, char *in) {
	int i;
	char ttok[MAXLABEL];
	sscanf(in, "%s", ttok); // arg, strcmp!

	if (strcmp(ttok, "amp") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (amparray[i] == NULL) {
				amparray[i] = m;
				break;
			}
		}
	}
	if (strcmp(ttok, "sig") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (sigarray[i] == NULL) {
				sigarray[i] = m;
				break;
			}
		}
	}
}

void msigout::disconnect(MODULES *m, char *in) {
	int i;
	char ttok[MAXLABEL];
	sscanf(in, "%s", ttok); // arg, strcmp!

	if (strcmp(ttok, "amp") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (amparray[i] == m) {
				amparray[i] = NULL;
				break;
			}
		}
	}
	if (strcmp(ttok, "sig") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (sigarray[i] == m) {
				sigarray[i] = NULL;
				break;
			}
		}
	}
}


// -------
menv::menv()
{
	// defaults
	curval = 1.0;
	timelength = 3.0;
	curenv = 0;
	index = 0.0;
	is_going = false;
	is_looping = 0;

	int nargs = 5;
	Arg *args = new Arg [nargs];
	args[0] = 0.0;
	args[1] = TABLELEN/2;
	args[2] = 1.0;
	args[3] = TABLELEN/2;
	args[4] = 0.0;

	fill_linebrk_table(args, nargs, envtable[curenv], TABLELEN);

	incr = (float)TABLELEN / (timelength * RTcmix::sr());

	float retval;
}

void menv::ping(float oamp)
{
	index = 0.0;
	is_going = true;
	overamp = oamp;
}

void menv::settimelength(float tv)
{
	timelength = tv;
	incr = (float)TABLELEN / (timelength * RTcmix::sr());
}

void menv::setloop(int l)
{
	is_looping = l;
}

float menv::getval()
{
	if (!is_calculated) {
		if (is_going) {
			curval = TablePField::Interpolate2ndOrder(envtable[curenv], TABLELEN, index) * overamp;
			index += incr;
			if ((int)index > TABLELEN) is_going = false;
		} else {
			if (is_looping == 1) {
				index = 0.0;
				is_going = true;
			}
		}
		is_calculated = true;
		retval = curval;
	}

	return retval;
}


// -------
mmoogvcf::mmoogvcf()
{
	for (int i = 0; i < NSLOTS; i++) {
		sigarray[i] = NULL;
		amparray[i] = NULL;
		resarray[i] = NULL;
		cfarray[i] = NULL;
	}

	// defaults
	amp = 1.0;
	centerfreq = 1000.0;
	resonance = 0.7;
	b0 = b1 = b2 = b3 = b4 = 0.0;

	// make_coefficients
	float tfreq = 2.0 * centerfreq / RTcmix::sr();
	q = 1.0 - tfreq;
	p = tfreq + (0.8 * tfreq * q);
	f = p + p - 1.0;
	q = resonance * (1.0 + 0.5 * q * (1.0 - q + 5.6 * q * q));
}

void mmoogvcf::setamp(float a)
{
	amp = a;
}

void mmoogvcf::setcfreq(float fval) {
	centerfreq = fval;

	// make_coefficients
	float tfreq = 2.0 * centerfreq / RTcmix::sr();
	q = 1.0 - tfreq;
	p = tfreq + (0.8 * tfreq * q);
	f = p + p - 1.0;
	q = resonance * (1.0 + 0.5 * q * (1.0 - q + 5.6 * q * q));
}

void mmoogvcf::setres(float rval) {
	if (rval > 15.0) {
		rval = 15.0;
		printf("WARNING: resonance too high, resetting to 15.0\n");
	}

	resonance = rval;

	// make_coefficients
	float tfreq = 2.0 * centerfreq / RTcmix::sr();
	q = 1.0 - tfreq;
	p = tfreq + (0.8 * tfreq * q);
	f = p + p - 1.0;
	q = resonance * (1.0 + 0.5 * q * (1.0 - q + 5.6 * q * q));
}

float mmoogvcf::getval() {
	if (!is_calculated) {

		// calculate any cfreq control
		float accum = 0.0;
		for (int i = 0; i < NSLOTS; i++) {
			if (cfarray[i] != NULL)
				accum += cfarray[i]->getval();
		}
		if (accum < 0.0) accum = -accum; // guard against negative cfreqs

		// calculate any resonance control
		float raccum = 0.0;
		for (int i = 0; i < NSLOTS; i++) {
			if (resarray[i] != NULL)
				raccum += resarray[i]->getval();
		}
		if (raccum < 0.0) raccum = -raccum; // guard against negative resons

		if (centerfreq < 50.0) centerfreq = 50.0;
		if (centerfreq > 15000.0) centerfreq = 15000.0;

		float basecenterfreq = centerfreq;
		float baseresonance = resonance;

		resonance += raccum; // this will be in setcfreq then
		if (resonance > 15.0) resonance = 15.0; // guard against too high reson

		setcfreq(centerfreq + accum); // setcfreq uses resonance too

		// reset to base values for next go-round
		centerfreq = basecenterfreq;
		resonance = baseresonance;

		accum = 0.0;

		for (int i = 0; i < NSLOTS; i++) {
		if (sigarray[i] != NULL)
			accum += sigarray[i]->getval();
		}

		float insig = accum;
		insig -= q * b4;           // feedback

		// four cascaded onepole filters (bilinear transform)
         float t1 = b1;  b1 = (insig + b0)  * p - b1 * f;
         float t2 = b2;  b2 = (b1 + t1)  * p - b2 * f;
         t1 = b3;        b3 = (b2 + t2)    * p - b3 * f;
                     b4 = (b3 + t1)  * p - b4 * f;
         b4 = b4 - b4 * b4 * b4 * 0.166667;           // clipping
         b0 = insig;

		accum = 0.0;

		// calculate any amp control
		for (int i = 0; i < NSLOTS; i++) {
			if (amparray[i] != NULL)
				accum += amparray[i]->getval();
		}

		retval =  b4 * (amp + accum);
		is_calculated = true;
	}
	return retval;
}

void mmoogvcf::connect(MODULES *m, char *in) {
	int i;
	char ttok[MAXLABEL];
	sscanf(in, "%s", ttok); // arg, strcmp!

	if (strcmp(ttok, "sig") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (sigarray[i] == NULL) {
				sigarray[i] = m;
				break;
			}
		}
	}

	if (strcmp(ttok, "amp") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (amparray[i] == NULL) {
				amparray[i] = m;
				break;
			}
		}
	}

	if (strcmp(ttok, "reson") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (resarray[i] == NULL) {
				resarray[i] = m;
				break;
			}
		}
	}

	if (strcmp(ttok, "cfreq") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (cfarray[i] == NULL) {
				cfarray[i] = m;
				break;
			}
		}
	}
}

void mmoogvcf::disconnect(MODULES *m, char *in) {
	int i;
	char ttok[MAXLABEL];
	sscanf(in, "%s", ttok); // arg, strcmp!

	if (strcmp(ttok, "sig") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (sigarray[i] == m) {
				sigarray[i] = NULL;
				break;
			}
		}
	}

	if (strcmp(ttok, "amp") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (amparray[i] == m) {
				amparray[i] = NULL;
				break;
			}
		}
	}

	if (strcmp(ttok, "reson") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (resarray[i] == m) {
				resarray[i] = NULL;
				break;
			}
		}
	}

	if (strcmp(ttok, "cfreq") == 0) {
		for (i = 0; i < NSLOTS; i++) {
			if (cfarray[i] == m) {
				cfarray[i] = NULL;
				break;
			}
		}
	}
}
