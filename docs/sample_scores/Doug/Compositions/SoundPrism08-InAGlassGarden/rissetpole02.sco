include tables.info
include audio.info
include spacechain.info
include primes.info

load("MULTIWAVE")
load("HOLO")

sFudgeDiff = 0;

if (?seed) {
	srand($seed);
}

list gAmpCurves;
gEnv = maketable("expbrk", 1000, 1.0, 750, 1.0, 250, 0.001);

handle rissetball(float dur, list ampcurves, float freq)
{
	inst = makeinstrument("MULTIWAVE", 0, dur, 1.0, wavet,
		freq * .56 + sFudgeDiff, ampcurves[0]*gEnv, irand(0, 359), 0,
		freq * .56 + 1 + sFudgeDiff, ampcurves[1]*gEnv * .67, irand(0, 359), 0,
		freq * .92 + sFudgeDiff, ampcurves[2]*gEnv, irand(0, 359), 0,
		freq * .92 + 1.7 + sFudgeDiff, ampcurves[3]*gEnv * 1.8, irand(0, 359), 0,
		freq * 1.19 + sFudgeDiff, ampcurves[4]*gEnv * 2.67, irand(0, 359), 0,
		freq * 1.7 + sFudgeDiff, ampcurves[5]*gEnv * 1.67, irand(0, 359), 0,
		freq * 2 + sFudgeDiff, ampcurves[6]*gEnv * 1.46, irand(0, 359), 0, 
		freq * 2.74 + sFudgeDiff, ampcurves[7]*gEnv * 1.33, irand(0, 359), 0,
		freq * 3 + sFudgeDiff, ampcurves[8]*gEnv * 1.33, irand(0, 359), 0,
		freq * 3.76 + sFudgeDiff, ampcurves[9]*gEnv, irand(0, 359), 0,
		freq * 4.07 + sFudgeDiff, ampcurves[10]*gEnv * 1.33, irand(0, 359), 0
	);
	return inst;
}

rvbtime = 1.4
mike_dist = 2.0;

configureRoom(47,65,54,10,rvbtime,mike_dist);

configureFrontInstrument("MULTIWAVE", 0);



start = 0;
end = 0;

endamp = 0.1;
minamp = 0.5;
maxamp = 1.0;

degreesPerSecond = 13.0;

setSpace(kForeground);

for (n = 0; n < 25; n += 1) {

	sFudgeDiff  = pickrand(-15, 11, 27, 31, 42);
	// a sine wave with extra harmonics for more complex bell sound
	wavet = maketable("wave", 5000, irand(0.9, 1.0), irand(0.01, 0.1), irand(0.01, 0.1))

	dur = irand(1, 3.6);

	// each frequency component in the risset tone gets its own random gain curve
	gAmpCurves = {
		makeline({0, 0, trand(10,30), endamp, trand(31, 60), irand(minamp, maxamp), trand(61,80), endamp, 100, 0}),
		makeline({0, 0, trand(10,30), endamp, trand(31, 60), irand(minamp, maxamp), trand(61,80), endamp, 100, 0}),
		makeline({0, 0, trand(10,30), endamp, trand(31, 60), irand(minamp, maxamp), trand(61,80), endamp, 100, 0}),
		makeline({0, 0, trand(10,30), endamp, trand(31, 60), irand(minamp, maxamp), trand(61,80), endamp, 100, 0}),
		makeline({0, 0, trand(10,30), endamp, trand(31, 60), irand(minamp, maxamp), trand(61,80), endamp, 100, 0}),
		makeline({0, 0, trand(10,30), endamp, trand(31, 60), irand(minamp, maxamp), trand(61,80), endamp, 100, 0}),
		makeline({0, 0, trand(10,30), endamp, trand(31, 60), irand(minamp, maxamp), trand(61,80), endamp, 100, 0}),
		makeline({0, 0, trand(10,30), endamp, trand(31, 60), irand(minamp, maxamp), trand(61,80), endamp, 100, 0}),
		makeline({0, 0, trand(10,30), endamp, trand(31, 60), irand(minamp, maxamp), trand(61,80), endamp, 100, 0}),
		makeline({0, 0, trand(10,30), endamp, trand(31, 60), irand(minamp, maxamp), trand(61,80), endamp, 100, 0}),
		makeline({0, 0, trand(10,30), endamp, trand(31, 60), irand(minamp, maxamp), trand(61,80), endamp, 100, 0})
	};
	// Play the metainstrument at frequency chosen from lowest portion of prime list
	inst = rissetball(dur, gAmpCurves, primeFrequencies[25+trand(0, len(primeFrequencies)/4)]);
	setFrontInstrument(inst);
	startangle = irand(0, 360);
	roomMoveArc(start, 0, dur, 10000, startangle, startangle + (degreesPerSecond * dur));
	start += irand(.376, 1.777);
	end = start;
}


roomRun(end, 0.53);



