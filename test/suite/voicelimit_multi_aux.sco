// voicelimit_multi_aux.sco - Stress test for independent bus parallelism.
// Spreads voices across 4 aux buses, each with its own MIX -> out.
//
// Usage:  CMIX < voicelimit_multi_aux.sco
//    or:  CMIX --voices=4096 < voicelimit_multi_aux.sco

set_option("buffer_count=5");
rtsetparams(44100, 1, 4096)
load("WAVETABLE")
load("MIX")

float voice_limit;

if (?voices) {
	voice_limit = $voices;
}
else {
	voice_limit = 10000;
}

printf("Starting %d voices spread across 4 aux buses\n", voice_limit);
print_off()

amp = 0.0025
dur = 10.0

wave = maketable("wave", 1000, 1., 0.3, 0.2)
env = maketable("line", 1000, 0,0, 1,1, 99,1,100,0)

// Distribute voices across 4 aux buses
start = 0
for (i = 0; i < voice_limit; i += 1) {
   auxbus = i % 4
   bus_config("WAVETABLE", "aux " + auxbus + " out")
   freq = irand(100, 1000)
   WAVETABLE(start, dur, amp*30000*env, freq, wave, 0);
   start += 0.0001
}

// Each aux bus gets its own MIX -> mono out
for (auxbus = 0; auxbus < 4; auxbus += 1) {
   bus_config("MIX", "aux " + auxbus + " in", "out 0")
   MIX(0, 0, dur + 1, 0.25, 0)
}
