include /Users/dscott/studies/soundprism08/tables.info
include /Users/dscott/studies/soundprism08/audio.info
include /Users/dscott/studies/soundprism08/spacechain.info
include /Users/dscott/studies/soundprism08/primes.info

load("FILTERBANK")
load("HOLO")

//srand(3234234);

gRingDown = 6.0
gBWFactor = 0.003

handle rissetfilter(float inskip, float dur, float freq)
{
	inst = makeinstrument("FILTERBANK", 0, inskip, dur, 1.0, gRingDown, 0, 0,
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

rtinput("dust2.wav");
configureFrontInstrument("FILTERBANK", 1);


rvbtime = 1.3
mike_dist = 2.0;

configureRoom(47,65,54,10,rvbtime,mike_dist);

setSpace(kBackground);

indur = DUR();
start = 0;
end = 0;

numfrequencies = len(primeFrequencies);

while (end < 75) {
	dur = irand(1.4, 4.7);
	start_angle = irand(-60, 60);
	delta_angle = 45 / dur;
	inskip = irand(0, indur-dur);
	gBWFactor = irand(0.001, 0.006);
	// Play the metainstrument at frequency chosen from higher portion of prime list
	inst = rissetfilter(inskip, dur, 1.5*primeFrequencies[numfrequencies/2 + trand(0, numfrequencies/2)]);
	setFrontInstrument(inst);
	roomMoveArc(start, 0, dur, 15, start_angle, start_angle + delta_angle);
	end = start + dur;
	start += irand(0.3,1.2);
	start_angle += delta_angle;
}


roomRun(end+rvbtime, 0.3);



