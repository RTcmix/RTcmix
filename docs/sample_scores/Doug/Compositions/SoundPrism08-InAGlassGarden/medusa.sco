include tables.info
include audio.info
include spacechain.info
include primes.info

load("MULTIWAVE")
load("HOLO")

//srand(3234234);

gEnv = makeline({0,0, 60,1, 1000,1, 2000,0});

handle medusa(float dur, list ampcurves, float baseFreq, list freqcurves)
{
	inst = makeinstrument("MULTIWAVE", 0, dur, 1.0, wavet,
		freqcurves[0] * .56 * baseFreq, ampcurves[0]*gEnv, irand(0, 359), 0,
		freqcurves[1] * .56 * baseFreq + 1, ampcurves[1]*gEnv * .67, irand(0, 359), 0,
		freqcurves[2] * .92 * baseFreq, ampcurves[2]*gEnv, irand(0, 359), 0,
		freqcurves[3] * .92 * baseFreq + 1.7, ampcurves[3]*gEnv * 1.8, irand(0, 359), 0,
		freqcurves[4] * 1.19 * baseFreq, ampcurves[4]*gEnv * 2.67, irand(0, 359), 0,
		freqcurves[5] * 1.7 * baseFreq, ampcurves[5]*gEnv * 1.67, irand(0, 359), 0,
		freqcurves[6] * 2 * baseFreq, ampcurves[6]*gEnv * 1.46, irand(0, 359), 0, 
		freqcurves[7] * 2.74 * baseFreq, ampcurves[7]*gEnv * 1.33, irand(0, 359), 0,
		freqcurves[8] * 3 * baseFreq, ampcurves[8]*gEnv * 1.33, irand(0, 359), 0,
		freqcurves[9] * 3.76 * baseFreq, ampcurves[9]*gEnv, irand(0, 359), 0,
		freqcurves[10] * 4.07 * baseFreq, ampcurves[10]*gEnv * 1.33, irand(0, 359), 0
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

minamp = 0.15;
maxamp = 1.0;

for (n = 0; n < 15; n += 1) {
	// a sine wave with extra harmonics for more complex bell sound
	wavet = maketable("wave", 5000, irand(0.8, 1.0), irand(0.1, 0.6), irand(0.1, 0.5))
	dur = irand(9, 13);

	minpoints = 3
	maxpoints = trand(4, 11);

	minfreqpoints = 3
	maxfreqpoints = trand(4, 17);
	freqMult = irand(1.1, 1.5);
	
	numpasses = trand(1, 3);
	
	for (count = 0; count < numpasses; count += 1) {
	
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
		baseFreq = primeFrequencies[trand(25, 65)];
	
		inst = medusa(dur, ampCurves, baseFreq, freqCurves);
		setFrontInstrument(inst);
		roomPlaceRandom(start, 0, dur, 10000);
	}
	
	start += (dur + irand(2,5));
	end = start + dur;
}


roomRun(end, 0.3);



