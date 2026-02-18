// voicelimit_aux.sco - Like voicelimit.sco but routes all WAVETABLEs
// through aux bus 0 via MIX, testing parallel TO_AUX execution via
// InstrumentBus phased production model.
//
// Usage:  CMIX < voicelimit_aux.sco
//    or:  CMIX --voices=500 < voicelimit_aux.sco

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

printf("Starting %d voices routed through aux 0\n", voice_limit);
print_off()

amp = 0.0025
dur = 10.0

wave = maketable("wave", 1000, 1., 0.3, 0.2)
env = maketable("line", 1000, 0,0, 1,1, 99,1,100,0)

// All WAVETABLEs write to aux bus 0 (TO_AUX)
bus_config("WAVETABLE", "aux 0 out")

start = 0
for (i = 0; i < voice_limit; i += 1) {
   freq = irand(100, 1000)
   WAVETABLE(start, dur, amp*30000*env, freq, wave, 0);
   start += 0.0001
}

// Single MIX reads from aux 0 and writes to mono output (TO_OUT)
bus_config("MIX", "aux 0 in", "out 0")
MIX(0, 0, dur + 1, 1, 0)
