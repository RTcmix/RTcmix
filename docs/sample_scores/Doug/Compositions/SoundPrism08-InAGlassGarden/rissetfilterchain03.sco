include tables.info
include audio.info
include spacechain.info
include primes.info

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
maxend = 30;
primeOffset = trand(300,450);
minoffset = 0.004;
maxoffset = 0.13;
gain = 6;
durfactor = 1.0;
maxbwfactor = 0.007;

anglerange = { -35, -30 };

while (end < maxend) {
	dur = durfactor * irand(0.4, 2.2);
	gBWFactor = irand(0.00056, maxbwfactor);
	// Play the metainstrument at frequency chosen from lowest portion of prime list
	inst = rissetfilter(0, dur, primeFrequencies[primeOffset+trand(0, len(primeFrequencies)/1.5)]);
	setFrontInstrument(inst);
	roomPlaceAngleRange(start, 0, dur, gain, anglerange[0], anglerange[1]);
	start += irand(minoffset,maxoffset);
	end = start + dur;
	anglerange = anglerange + irand(-30, 45);
	minoffset *= 1.002;
	maxoffset *= 1.011;
	gain *= 0.98;
	durfactor += 0.0025;
	maxbwfactor *= 0.99;
}


roomRun(end, 0.3);



