rtsetparams(44100,2)
load("/home/jwmatthys/my_rtcmix/tuning/libtuning.so")
load("MCLAR")
print_off()

/*	libtuning: custom tunings

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
			want to tune to A=442.
	
	you can change the diapason (the base frequency)
	with the diapason command. This:
	
	diapason(440, 8.00)
	
	declares 440 Hz to be oct.pc 8.00 in the new tuning. if the first value is
	less than 18, it is interpreted as an RTcmix oct.pc value. So:

	diapason (8.00, 8.00)
	
	declares equal-tempered middle C (261.625565301 Hz) to be 8.00. This
	is the default value anyway.*/

diapason(8.00,7.00) // This should transpose everything up octave by declaring that
	//the pitch we usually associate with 8.00 is now 7.00

env = maketable("line",1000,0,0,1,1,9,1,10,0)

start = 0

// Equal Temperament: Scale and I-V7-I
for (i=0; i<13; i+=1)
{
	freq = eqtemp(7+i/100)
	start += 0.5
	MCLAR(start,0.5,10000*env,freq,0.4,0.4,0)
}

MCLAR(start+1,1,10000*env,eqtemp(7.00),0.4,0.4,0)
MCLAR(start+1,1,10000*env,eqtemp(7.04),0.4,0.4,0)
MCLAR(start+1,1,10000*env,eqtemp(7.07),0.4,0.4,0)
MCLAR(start+1,1,10000*env,eqtemp(8.00),0.4,0.4,0)
MCLAR(start+2,1,10000*env,eqtemp(6.07),0.4,0.4,0)
MCLAR(start+2,1,10000*env,eqtemp(7.05),0.4,0.4,0)
MCLAR(start+2,1,10000*env,eqtemp(7.07),0.4,0.4,0)
MCLAR(start+2,1,10000*env,eqtemp(7.11),0.4,0.4,0)
MCLAR(start+3,1,10000*env,eqtemp(6.00),0.4,0.4,0)
MCLAR(start+3,1,10000*env,eqtemp(7.04),0.4,0.4,0)
MCLAR(start+3,1,10000*env,eqtemp(7.07),0.4,0.4,0)
MCLAR(start+3,1,10000*env,eqtemp(8.00),0.4,0.4,0)
start += 4

// Pure Pythagorean diatonic scale
for (i=0; i<8; i+=1)
{
	freq = pythag(7+i/100)
	start += 0.5
	MCLAR(start,0.5,10000*env,freq,0.4,0.4,0)
}

MCLAR(start+1,1,10000*env,pythag(7.00),0.4,0.4,0)
MCLAR(start+1,1,10000*env,pythag(7.02),0.4,0.4,0)
MCLAR(start+1,1,10000*env,pythag(7.04),0.4,0.4,0)
MCLAR(start+1,1,10000*env,pythag(8.00),0.4,0.4,0)
MCLAR(start+2,1,10000*env,pythag(6.04),0.4,0.4,0)
MCLAR(start+2,1,10000*env,pythag(7.03),0.4,0.4,0)
MCLAR(start+2,1,10000*env,pythag(7.04),0.4,0.4,0)
MCLAR(start+2,1,10000*env,pythag(7.06),0.4,0.4,0)
MCLAR(start+3,1,10000*env,pythag(6.00),0.4,0.4,0)
MCLAR(start+3,1,10000*env,pythag(7.02),0.4,0.4,0)
MCLAR(start+3,1,10000*env,pythag(7.04),0.4,0.4,0)
MCLAR(start+3,1,10000*env,pythag(8.00),0.4,0.4,0)
start += 4


// Chromatic Quarter Comma Mean Tone Temperament
for (i=0; i<13; i+=1)
{
	freq = meantone(7+i/100)
	start += 0.5
	MCLAR(start,0.5,10000*env,freq,0.4,0.4,0)
}

MCLAR(start+1,1,10000*env,meantone(7.00),0.4,0.4,0)
MCLAR(start+1,1,10000*env,meantone(7.04),0.4,0.4,0)
MCLAR(start+1,1,10000*env,meantone(7.07),0.4,0.4,0)
MCLAR(start+1,1,10000*env,meantone(8.00),0.4,0.4,0)
MCLAR(start+2,1,10000*env,meantone(6.07),0.4,0.4,0)
MCLAR(start+2,1,10000*env,meantone(7.05),0.4,0.4,0)
MCLAR(start+2,1,10000*env,meantone(7.07),0.4,0.4,0)
MCLAR(start+2,1,10000*env,meantone(7.11),0.4,0.4,0)
MCLAR(start+3,1,10000*env,meantone(6.00),0.4,0.4,0)
MCLAR(start+3,1,10000*env,meantone(7.04),0.4,0.4,0)
MCLAR(start+3,1,10000*env,meantone(7.07),0.4,0.4,0)
MCLAR(start+3,1,10000*env,meantone(8.00),0.4,0.4,0)
start += 4

// 5-Limit Just Intonation: Scale and I-V-I
for (i=0; i<13; i+=1)
{
	freq = just(7+i/100)
	start += 0.5
	MCLAR(start,0.5,10000*env,freq,0.4,0.4,0)
}

MCLAR(start+1,1,10000*env,just(7.00),0.4,0.4,0)
MCLAR(start+1,1,10000*env,just(7.04),0.4,0.4,0)
MCLAR(start+1,1,10000*env,just(7.07),0.4,0.4,0)
MCLAR(start+1,1,10000*env,just(8.00),0.4,0.4,0)
MCLAR(start+2,1,10000*env,just(6.07),0.4,0.4,0)
MCLAR(start+2,1,10000*env,just(7.05),0.4,0.4,0)
MCLAR(start+2,1,10000*env,just(7.07),0.4,0.4,0)
MCLAR(start+2,1,10000*env,just(7.11),0.4,0.4,0)
MCLAR(start+3,1,10000*env,just(6.00),0.4,0.4,0)
MCLAR(start+3,1,10000*env,just(7.04),0.4,0.4,0)
MCLAR(start+3,1,10000*env,just(7.07),0.4,0.4,0)
MCLAR(start+3,1,10000*env,just(8.00),0.4,0.4,0)
start += 4

// LaMonte Young's 12-note scale from _The Well Tuned Piano_
for (i=0; i<12; i+=1)
{
	freq = young(7+i/100)
	start += 0.5
	MCLAR(start,0.5,10000*env,freq,0.4,0.4,0)
}

MCLAR(start+1,1,10000*env,young(7.00),0.4,0.4,0)
MCLAR(start+1,1,10000*env,young(7.04),0.4,0.4,0)
MCLAR(start+1,1,10000*env,young(7.07),0.4,0.4,0)
MCLAR(start+1,1,10000*env,young(8.00),0.4,0.4,0)
MCLAR(start+2,1,10000*env,young(6.07),0.4,0.4,0)
MCLAR(start+2,1,10000*env,young(7.05),0.4,0.4,0)
MCLAR(start+2,1,10000*env,young(7.07),0.4,0.4,0)
MCLAR(start+2,1,10000*env,young(7.11),0.4,0.4,0)
MCLAR(start+3,1,10000*env,young(6.00),0.4,0.4,0)
MCLAR(start+3,1,10000*env,young(7.04),0.4,0.4,0)
MCLAR(start+3,1,10000*env,young(7.07),0.4,0.4,0)
MCLAR(start+3,1,10000*env,young(8.00),0.4,0.4,0)
start += 4

// Harry Partch's 43-note scale
for (i=0; i<44; i+=1)
{
	freq = partch(7+i/100)
	start += 0.25
	MCLAR(start,0.25,10000*env,freq,0.4,0.4,0)
}
start +=1

/*	let's try defining a custom scale. Here's a 7-limit scale based on
	proportions with 5,7,or 9 in the denominator.
	I found here: http://www.huygens-fokker.org/bpsite/otherscales.html
	As is customary, all values are given as proportions starting at 1. */

create_scale(1/1, 10/9, 6/5, 9/7, 7/5, 14/9, 5/3, 9/5, 2)
	
/*	Notice that for this 8-note scale there are nine elements. The last
	element is very important, as it defines the harmonic span of your
	scale. For normal one-octave scales, this value will be 2, since each
	octave represents a double of frequency. But you can also create scales
	that repeat at values other than the octave. */

/*	I set the maximum number of elements to 64 notes to the scale, but if somehow
	that's not enough for you (!), you can increase it in line 22 of tuning.c */

for (i=0; i<9; i+=1)
{
	freq = myscale(7+i/100)
	start += 1
	MCLAR(start,1,10000*env,freq,0.4,0.4,0)
}
start +=2

// how about some chords?
srand()
for (i=0; i<7; i+=1)
{
	for (j=0; j<3; j+=1)
	{
		pitchclass = trand(0,15)
		pitch = 6+(pitchclass/100)
		freq = myscale(pitch)
		MCLAR(start,1,10000*env,freq,0.4,0.4,0)
	}
	start += 1
}

// here's the Bohlen-Pierce scale, a 13-note scale which repeats at the
// twelfth rather than the octave.

diapason(220,1.00) // set oct.pc 1.00 to 220 Hz
create_scale(1, 27/25, 25/21, 9/7, 7/5, 75/49, 5/3, 9/5, 49/25, 15/7, 7/3, 63/25, 25/9, 3)

for (i=0; i<14; i+=1)
{
	freq = myscale(1+i/100)
	start += 1
	MCLAR(start,1,10000*env,freq,0.4,0.4,0)
}
start += 2

// check out these chords. for such a strange scale, i find that the random chords here
// are quite consonant.
for (i=0; i<7; i+=1)
{
	for (j=0; j<3; j+=1)
	{
		pitchclass = trand(0,27)
		pitch = 1+pitchclass/100
		freq = myscale(pitch)
		MCLAR(start,1,10000*env,freq,0.4,0.4,0)
	}
	start += 1
}
