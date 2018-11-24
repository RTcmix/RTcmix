include /Users/dscott/studies/soundprism08/tables.info
include /Users/dscott/studies/soundprism08/audio.info
include /Users/dscott/studies/soundprism08/spacechain.info
include /Users/dscott/studies/soundprism08/primes.info

load("FILTERBANK")
load("HOLO")

if (?seed) {
	srand($seed);
}

gRingDown = 2.0
gBWFactor = irand(0.002, 0.02);

handle rissetfilter(float inskip, float dur, float freq)
{
	inst = makeinstrument("FILTERBANK", 0, inskip, dur, 4.0, gRingDown, 0, 0,
		freq * .56, gBWFactor, 1,
		freq * .56 + 1, gBWFactor, .67,
		freq * .92, gBWFactor, 1,
		freq * .92 + 1.7, gBWFactor, 1.8,
		freq * 1.19, gBWFactor, 2.67,
		freq * 1.7, gBWFactor, 1.67,
		freq * 2, gBWFactor, 1.46, 
		freq * 2.74, gBWFactor, 1.33,
		freq * 3, gBWFactor, 1.33,
		freq * 3.76, gBWFactor, 1,
		freq * 4.07, gBWFactor, 1.33
	);
	return inst;
}

rtinput("impulse.wav");
configureFrontInstrument("FILTERBANK", 1);


rvbtime = 1.3
mike_dist = 2.0;

configureRoom(47,65,54,10,rvbtime,mike_dist);

setSpace(kBackground);

start = 0;
end = 0;
maxend = irand(25, 45);
primeOffset = trand(200,300);

anglerange = { -35, -30 };

while (end < maxend) {
	dur = irand(0.4, 2.2);
	gBWFactor = irand(0.00056, 0.007);
	// Play the metainstrument at frequency chosen from lowest portion of prime list
	inst = rissetfilter(0, dur, primeFrequencies[primeOffset+trand(0, len(primeFrequencies)/1.5)]);
	setFrontInstrument(inst);
	roomPlaceAngleRange(start, 0, dur, 6, anglerange[0], anglerange[1]);
	start += irand(0.004,0.13);
	end = start + dur;
	anglerange = anglerange + irand(-30, 45);
}


roomRun(end, 0.3);



