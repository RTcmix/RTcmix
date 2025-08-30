// Test score for MIDI function calls and instruments

set_option("midi_outdevice = Internal MIDI Bus 2");
rtsetparams(48000, 2, 1024);
load("MIDI");

setup_midi();

start = 0;
dur = 1/8;
chan = 0;
pitches = { 8.00, 8.02, 8.04, 8.05, 8.07, 8.09, 8.11 };
vel = 0.5;
incr = 1/8;

CONTROLLER(0, 1, chan, 10, 0.5);
for (n = 0; n < 64; ++n) {
	NOTE(start, dur, chan, pickrand(pitches), vel);
	NOTE(start+irand(0.00, 0.005), dur, chan+1, pickrand(pitches), vel);
	start += incr;
}

