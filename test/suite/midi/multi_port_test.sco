load("MIDI");
rtsetparams(48000, 2);

setup_midi("Internal MIDI Bus 1", "Internal MIDI Bus 2");

// Now try sending notes on the first 16 channel port...
// and on the second 16 channel port...

for (start = 0; start < 10; start += 0.5) {
	NOTE(start, dur=0.5, chan=10, pitch=8.00, vel=1.0);
	NOTE(start, dur=0.5, chan=18, pitch=8.04, vel=1.0);
}
