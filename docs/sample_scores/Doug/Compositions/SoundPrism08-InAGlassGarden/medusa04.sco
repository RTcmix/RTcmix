include tables.info
include audio.info
include spacechain.info
include primes.info

load("MULTIWAVE")
load("HOLO")

if (?seed) {
	srand($seed);
}

gEnv = maketable("expbrk", 1000, 1, 100, 1.0, 500, 1.0, 400, 0.01);
gEnv2 = maketable("line", 1000, 0, 0, 10, 1, 50, 1, 60, 0);

handle medusa(float dur, list ampcurves, float baseFreq, list freqcurves)
{
	inst = makeinstrument("MULTIWAVE", 0, dur, 15.0, wavet,
		freqcurves[0] * .56 * baseFreq, ampcurves[0]*gEnv*gEnv2, irand(0, 359), 0,
		freqcurves[1] * .56 * baseFreq + 1, ampcurves[1]*gEnv*gEnv2 * .67, irand(0, 359), 0,
		freqcurves[2] * .92 * baseFreq, ampcurves[2]*gEnv*gEnv2, irand(0, 359), 0,
		freqcurves[3] * .92 * baseFreq + 1.7, ampcurves[3]*gEnv*gEnv2 * 1.8, irand(0, 359), 0,
		freqcurves[4] * 1.19 * baseFreq, ampcurves[4]*gEnv*gEnv2 * 2.67, irand(0, 359), 0,
		freqcurves[5] * 1.7 * baseFreq, ampcurves[5]*gEnv*gEnv2 * 1.67, irand(0, 359), 0,
		freqcurves[6] * 2 * baseFreq, ampcurves[6]*gEnv*gEnv2 * 1.46, irand(0, 359), 0, 
		freqcurves[7] * 2.74 * baseFreq, ampcurves[7]*gEnv*gEnv2 * 1.33, irand(0, 359), 0,
		freqcurves[8] * 3 * baseFreq, ampcurves[8]*gEnv*gEnv2 * 1.33, irand(0, 359), 0,
		freqcurves[9] * 3.76 * baseFreq, ampcurves[9]*gEnv*gEnv2, irand(0, 359), 0,
		freqcurves[10] * 4.07 * baseFreq, ampcurves[10]*gEnv*gEnv2 * 1.33, irand(0, 359), 0
	);
	return inst;
}

rvbtime = 1.3
mike_dist = 2.0;

configureRoom(47,65,54,10,rvbtime,mike_dist);

configureFrontInstrument("MULTIWAVE", 0);

setSpace(kMidground);

start = 0;
end = 0;

minamp = 0.10;
maxamp = 1.0;

// a sine wave with extra harmonics for more complex bell sound
wavet = maketable("wave", 5000, irand(0.9, 1.0), irand(0.05, 0.4), irand(0.02, 0.25))
dur = 33

minpoints = 3
maxpoints = trand(4, 11);

minfreqpoints = 3
maxfreqpoints = trand(4, 11);
freqMult = irand(1.4, 2.1);


// each frequency component in the risset tone gets its own random curve
freqCurves = {
	randtable(1/freqMult, freqMult, trand(minfreqpoints, maxfreqpoints)),
	randtable(1/freqMult, freqMult, trand(minfreqpoints, maxfreqpoints)),
	randtable(1/freqMult, freqMult, trand(minfreqpoints, maxfreqpoints)),
	randtable(1/freqMult, freqMult, trand(minfreqpoints, maxfreqpoints)),
	randtable(1/freqMult, freqMult, trand(minfreqpoints, maxfreqpoints)),
	randtable(1/freqMult, freqMult, trand(minfreqpoints, maxfreqpoints)),
	randtable(1/freqMult, freqMult, trand(minfreqpoints, maxfreqpoints)),
	randtable(1/freqMult, freqMult, trand(minfreqpoints, maxfreqpoints)),
	randtable(1/freqMult, freqMult, trand(minfreqpoints, maxfreqpoints)),
	randtable(1/freqMult, freqMult, trand(minfreqpoints, maxfreqpoints)),
	randtable(1/freqMult, freqMult, trand(minfreqpoints, maxfreqpoints))
};

// each frequency component in the risset tone gets its own random gain curve
ampCurves = {
	randtable(minamp, maxamp, trand(minpoints, maxpoints)),
	randtable(minamp, maxamp, trand(minpoints, maxpoints)),
	randtable(minamp, maxamp, trand(minpoints, maxpoints)),
	randtable(minamp, maxamp, trand(minpoints, maxpoints)),
	randtable(minamp, maxamp, trand(minpoints, maxpoints)),
	randtable(minamp, maxamp, trand(minpoints, maxpoints)),
	randtable(minamp, maxamp, trand(minpoints, maxpoints)),
	randtable(minamp, maxamp, trand(minpoints, maxpoints)),
	randtable(minamp, maxamp, trand(minpoints, maxpoints)),
	randtable(minamp, maxamp, trand(minpoints, maxpoints)),
	randtable(minamp, maxamp, trand(minpoints, maxpoints))
};
// Play the metainstrument at frequency chosen from lowest portion of prime list
baseFreq = primeFrequencies[trand(20, 65)];

inst = medusa(dur, ampCurves, baseFreq, freqCurves);
setFrontInstrument(inst);
roomPlaceRandom(start, 0, dur, 8000);

end = start + dur;


roomRun(end, 0.6);



