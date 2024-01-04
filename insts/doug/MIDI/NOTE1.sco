// Test score for NOTE instrument

// NOTE(start, duration, MIDI chan, pitch (oct pt pc), velocity (0-1.0)

set_option("midi_outdevice = Internal MIDI: Bus 2");
rtsetparams(48000, 2, 1024);
load("MIDI");

setup_midi();

start = 0;
dur = 1/4;
chan = 0;
pitch = 7.00;
vel = 0.5;
incr = 0.01;

for (n = 0; n < 8; ++n) {
	NOTE(start, dur, chan, pitch, vel);
	start += dur;
	pitch += incr;
	vel += 0.05;
}

