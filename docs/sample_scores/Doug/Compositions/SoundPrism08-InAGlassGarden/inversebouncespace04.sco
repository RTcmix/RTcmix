print_on(5)

include audio.info

load("WAVETABLE")
load("HOLO")

include tables.info
include space.info
include primes.info

if (?seed) {
	srand($seed);
}

set_bus_count(40);
init_busses(4);	// leave 0-3 available for connection to RVB

handle wavet;

float sFudgeDiff;
sBounceFactor = 1.0;

// risset just does the sine waves, all funneled into the current bus connecting WAVETABLE and DMOVE

float risset(float start, float dur, handle amp, float freq)
{
	WAVETABLE(start, dur, 		amp, freq * .56 + sFudgeDiff, 0, wavet)
	WAVETABLE(start, dur * .9, amp * .67, freq * .56 + 1 + sFudgeDiff, 0, wavet)
	WAVETABLE(start, dur * .65, amp, freq * .92 + sFudgeDiff, 0, wavet)
	WAVETABLE(start, dur * .55, amp * 1.8, freq * .92 + 1.7 + sFudgeDiff, 0, wavet)
	WAVETABLE(start, dur * .325, amp * 2.67, freq * 1.19 + sFudgeDiff, 0, wavet)
	WAVETABLE(start, dur * .35, amp * 1.67, freq * 1.7 + sFudgeDiff, 0, wavet)
	WAVETABLE(start, dur * .25, amp * 1.46, freq * 2 + sFudgeDiff, 0, wavet)
	WAVETABLE(start, dur * .2, amp * 1.33, freq * 2.74 + sFudgeDiff, 0, wavet)
	WAVETABLE(start, dur * .15, amp * 1.33, freq * 3 + sFudgeDiff, 0, wavet)
	WAVETABLE(start, dur * .1, amp, freq * 3.76 + sFudgeDiff, 0, wavet)
	WAVETABLE(start, dur * .075, amp * 1.33, freq * 4.07 + sFudgeDiff, 0, wavet)
	return start + dur;
}

float risset_bounce(float start, float totalDur, float dur, float gain, float freq, float spacing, float spacingfactor,
					float durfactor, float gainfactor)
{
	printf("start: %f, totalDur: %f, dur: %f, gain: %f, freq: %f, spacing: %f, spacingfactor: %f, durfactor: %f, gainfactor: %f\n",
		start, totalDur, dur, gain, freq, spacing, spacingfactor, durfactor, gainfactor);
	// just a sine wave (try extra harmonics for more complex bell sound)
	wavet = maketable("wave", 5000, irand(0.8, 1.0), irand(0.1, 0.7))
	// exponential amplitude envelope ("F2" in Dodge)
	env = maketable("expbrk", 1000,  1, 1000, .0000000002)
	lastStart = start + totalDur;
	end = 0;
	localstart = start;
	while (spacing < 0.75 && localstart < lastStart) {
		amp = gain * env
		end = max(end, risset(localstart, dur, amp, freq));
		localstart  += spacing
		spacing /= spacingfactor
		dur /= durfactor
		gain /= gainfactor
	}
	return end;
}

_glassEnergyGainTable = maketable("expbrk", "nonorm", 1000, 0.05, 1000, 1);		// lookup for gain
_glassEnergyDurTable = maketable("expbrk", "nonorm", 1000, 0.2, 1000, 2.0);			// factor for dur of hits
_glassEnergySpacingTable = maketable("expbrk", "nonorm", 1000, 0.1, 1000, 1.0);		// factor for spacing of hits

_glassMaterialDurTable = maketable("expbrk", "nonorm", 1000, 0.6, 1000, 4.0);		// lookup for dur
_glassMaterialGainTable = maketable("expbrk", "nonorm", 1000, 0.5, 1000, 1.5);		// lookup for gain

_glassMaterialGainFactorTable = maketable("expbrk", "nonorm", 1000, 0.5, 1000, 0.92);		// lookup for gain factor
_glassMaterialDurFactorTable = maketable("expbrk", "nonorm", 1000, 0.7, 1000, 0.95);		// lookup for dur factor
_glassMaterialSpacingFactorTable = maketable("expbrk", "nonorm", 1000, 0.6, 1000, 0.95);		// lookup for spacing factor

// energy: 1 = lightest possible hit, 100 = hardest possible hit
// material: 1 = dullest, least elastic, 100 = brightest, most elastic
// size: 1 = smallest, 100 = largest

kMaxGlassGain = 700;

float glass(float outskip, float totalDur, float energy, float material, float size)
{
	printf("outskip: %f, totalDur: %f, energy: %f, material: %f, size:%f\n", outskip, totalDur, energy, material, size);
	energy = min(max(1, energy), 100);
	material = min(max(1, material), 100);
	
	increment = 0.0005;
	dur = 0.007;
	gain = 0.01;
	
	durfactor = sampleline(_glassMaterialDurFactorTable, material/100.0);
	gainfactor = sampleline(_glassMaterialGainFactorTable, material/100.0);
	incrfactor = sampleline(_glassMaterialSpacingFactorTable, material/100.0);
	
	increment *= incrfactor;

	increment *= sBounceFactor;
	
	frqindex = trunc(size * len(primeFrequencies) / 100);
	freq = primeFrequencies[frqindex];
	endtime = risset_bounce(outskip, totalDur, dur, gain, freq, increment, incrfactor, durfactor, gainfactor);
	return endtime;
}

// Override default if we need to stick HOLO after RVB

if (holo) {
	bus_config("RVB", "aix0-3", "aux 30-31 out");
	bus_config("HOLO", "aux 30-31 in", "out0-1");
}

rvbtime = 1.3
mike_dist = 2.0;

setFrontInstrument("WAVETABLE", 0);

configureRoom(47,65,54,10,rvbtime,mike_dist);

setSpace(kMidground);

sBounceFactor = 0.999;

sFudgeDiff  = trunc(pickrand(-400, -312, -254, 213, 472, 513) * 1.5);
energy = trand(2, 12);	// how "hard" the glass is hit
hardness = irand(78, 99);
	
size = pickrand(57, 63, 65, 71, 87, 94);
start = 0
startAngle = irand(-70, 70);
endAngle = irand(-70, 70);

dur = irand(5, 15.5);

endtime = glass(start, dur, energy, hardness, size);
roomMoveArc(start, 0, endtime, 20, startAngle, endAngle);

roomRun(endtime, 0.2);

if (holo) {
	HOLO(0, 0, endtime, 1.0);
}
