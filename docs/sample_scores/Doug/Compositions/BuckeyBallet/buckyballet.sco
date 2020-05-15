load("MPLACE");
include mike.info
include audio.info
include room.info
include tables.info

load("STRUM");
load("MOOGVCF")
load("WAVESHAPE")
load("CHAIN");

bus_config("START","chain 0 out");
bus_config("MOOGVCF", "chain 0 in", "chain 1 out");
bus_config("MPLACE", "chain 1 in", "chain 2-5 out");
bus_config("CHAIN", "aux 0-3 out");
bus_config("RVB","aux 0-3 in","out 0-1");


space(dist_front,dist_right,dist_rear,dist_left,height/4,abs_fac,rvbtime)
//set_attenuation_params(4, 50, 2);
dist_mikes = 5;

float chooseAndClear(list inputArray, list slots)
{
	length = len(inputArray);
	index = trand(0, length);
	count = 0;
	while (slots[index] == 1) { 
		index = trand(0, length);
		if (count > 2 * length) {
			for (i = 0; i < length; i += 1) slots[i] = 0;	// reset all slots
		}
		count += 1;
	}
	slots[index] = 1;
	value = inputArray[index];
	return value;
}

global_cutoff_factor = 1;

float fstrum(float outskip, float dur, float pitch, float intensity, float rho, float theta)
{
	fund_decay = irand(dur * 0.4, dur * 0.6);
	nf_decay = irand(0.01, 0.08);
	cf = 60 + (5000 * intensity / 10);
	cf *= global_cutoff_factor;
	res = 0.05;
	amp = 10000 + (20000 * intensity / 10);
	handle start;
	start = makeinstrument("START", 0, dur, pitch, fund_decay, nf_decay, amp, 1, 0);
	moog = makeinstrument("MOOGVCF", 0, 0, dur, 20, inchan=0, pan=0, bypass=0, cf, res)
	place = makeinstrument("MPLACE", 0,0,dur,12,rho,theta,dist_mikes);
	return CHAIN(outskip, dur, 3, start, moog, place);
}

float sum(list array)
{
	total = 0;
	for (i = 0; i < len(array); i += 1) {
		total += array[i];
	}
	return total;
}

c_hexachord = 1;
measure_counts = { 8, 4, 7, 5, 6, 6, 5, 7, 4, 8, 3, 9 };
count_index = 0;

c_array = { 0.00, 0.02, 0.04, 0.05, 0.07, 0.09 };
c_slots = { 0, 0, 0, 0, 0, 0 };
fs_array = { 0.06, 0.08, 0.10, 0.11, 1.01, 1.03 };
fs_slots = { 0, 0, 0, 0, 0, 0 };

beats = { 0, 2, 1, 0, 2, 1, 1, 0, 2, 1, 1, 1 };
numbeats = len(beats);
beat = 0;
soundingbeat = 0;
measure = 0;

start = 0;
incr = 0.15;

num_measures = sum(measure_counts);

// accomp
for (notes = 0; notes < num_measures * numbeats; notes += 1) {
	if (beats[beat]) {
		float strength;
		if (beats[beat] == 2) { strength = 9; }
		else { strength = 4 }
		dur = 1;
		if (c_hexachord == 1) {
			fstrum(start, dur, 8 + chooseAndClear(c_array, c_slots), strength, 20, 360 * soundingbeat / numbeats);
			fstrum(start, dur, 8 + chooseAndClear(c_array, c_slots), strength, 20, 180 + (360 * soundingbeat / numbeats));
		}
		else {
			strength *= 0.6;
			fstrum(start+irand(-0.005, 0.005), dur, 8 + chooseAndClear(fs_array, fs_slots), strength, 15, 120 * soundingbeat / numbeats);
			fstrum(start+irand(-0.005, 0.005), dur, 8 + chooseAndClear(fs_array, fs_slots), strength, 15, 240 + (360 * soundingbeat / numbeats));
			fstrum(start+irand(-0.005, 0.005), dur, 8 + chooseAndClear(fs_array, fs_slots), strength, 15, 0 + (360 * soundingbeat / numbeats));
		}
		soundingbeat += 1;
	}
	beat = (beat + 1) % numbeats;
	if (beat == 0) {
		measure += 1;
	}
	if (measure == measure_counts[count_index]) {
		count_index = (count_index + 1) % len(measure_counts);
		measure = 0;
		c_hexachord = (c_hexachord == 0);
	}
	start += incr;
}

acomp_end = start;

bstart = incr * 2 * numbeats;
bindex = 0;

basslength = { 5, 4, 3 };
numbasslength = len(basslength);
measure = 2;
count_index = 0;

global_cutoff_factor = 0.46;		// dull down the bass
// bass
while (bstart < acomp_end-1) {
	dur = 2;
	if (c_hexachord) {
		fstrum(bstart, dur, 6 + chooseAndClear(c_array, c_slots), 9, 20, 360 * bindex / numbeats);
	}
	else {
		fstrum(bstart, dur, 5 + chooseAndClear(fs_array, c_slots), 5, 15, 180 * bindex / numbeats);
	}
	bstart += basslength[bindex] * incr;
	bindex = (bindex + 1) % numbasslength;
	if (bindex == 0) {
		measure += 1;
	}
	if (measure == measure_counts[count_index]) {
		count_index = (count_index + 1) % len(measure_counts);
		measure = 0;
		c_hexachord = (c_hexachord == 0);
	}
}

bus_config("WAVESHAPE", "chain 0 out");
bus_config("MPLACE", "chain 0 in", "chain 2-5 out");

wavetable = maketable("wave", 10000, "sine")
transferfunc = maketable("cheby", 10000, 0.9, 0.3, -0.2, 0.6, -0.7)
indexguide = maketable("line", 10000, 0,0, 2.5,1, 7,0)

viberamp = maketable("line", 10000, 0, 0, 1, 0.2, 3, 1, 5, 0);
vibe = makeLFO("tri", 3.8, 0.007)

rho = 30;
theta = 24;
amp = 30000;
env = maketable("line", 10000, 0,0, 1,1, 3, 1, 7,0);

wstart = numbeats * incr * 4;
oldpitch = 0;

while (wstart < acomp_end - 3) {
	float pitch;
	float in_c, total;
	in_c = 0;
	for (m = 0; m < len(measure_counts) && total < wstart; m += 1) {
		total += measure_counts[m] * numbeats * incr;
		in_c = (in_c == 0);
	}
	if (in_c) {
		pitch = pickrand(9.11, 9.04, 9.05, 9.02, 9.07);
		if (pitch == oldpitch) {
			pitch = pickrand(9.11, 9.04, 9.05, 9.02, 9.07);
		}
		oldpitch = pitch;
	}
	else {
		pitch = pickrand(10.01, 9.05, 9.10, 9.08, 9.03);		// 6, 8, 10, 11, 13
		if (pitch == oldpitch) {
			pitch = pickrand(10.01, 9.05, 9.10, 9.08, 9.03);
		}
		oldpitch = pitch;
	}
	rbeats = trunc(pickrand(pickrand(3, 3, 3, 3, 4, 4, 4), 12));
	skip = rbeats * incr;
	dur = skip + 1;
	if (chance(2, 3)) {
		shape = makeinstrument("WAVESHAPE", 0, dur, pitch + (vibe * viberamp), 0, 1, amp * env, pan=0, wavetable, transferfunc, indexguide)
		place = makeinstrument("MPLACE", 0,0,dur,100,rho,theta,dist_mikes);
		CHAIN(wstart, dur, 2, shape, place);
	}
	theta *= pickrand(-1, 1);
	wstart += skip;
}


end_time = start+2;
RVB(0, 0, end_time+4, 0.2);
