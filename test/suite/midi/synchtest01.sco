// Test score for MIDI function calls and instruments

set_option("midi_outdevice = Internal MIDI Bus 2");
print_on(6);
rtsetparams(48000, 2, 2048);
load("MIDI");
load("WAVETABLE");

setup_midi();

start = 0;
dur = 0.04;
gain = 0.9;
chan = 0;
pitch = 8.07;
vel = 0.9;
incr = 1;

wavetable = maketable("wave", 1000, 1, 0.3, 0.2)
wgain = 10000 * maketable("line", 1000, 0,1, 1,1, 3,1, 4,0)

tempo(0, 120);

CONTROLLER(0, 1, chan, 10, 1);

for (n = 0; n < 600; ++n) {
	time = tb(start);
	WAVETABLE(start+0.02, dur, wgain, cpspch(pitch), pan=1, wavetable)
	NOTE(start, dur, chan, pitch+0.06, vel);
	start += incr;
}

