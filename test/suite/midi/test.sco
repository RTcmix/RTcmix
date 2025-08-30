// Test score for MIDI function calls and instruments

set_option("midi_outdevice = Internal MIDI Bus 1","require_sample_rate=false");
rtsetparams(48000, 2, 1024);
load("MIDI");

setup_midi();

start = 0;
dur = 1/4;
chan = 0;
pitch = 6.07;
vel = 0.5;
incr = 0.01;

for (n = 0; n < 8; ++n) {
	NOTE(start, dur, chan, pitch, vel);
	start += dur;
	pitch += incr;
}

