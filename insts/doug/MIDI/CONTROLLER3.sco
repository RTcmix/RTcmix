// Test score for CONTROLLER instrument

set_option("midi_outdevice = Internal MIDI Bus 2");
rtsetparams(48000, 2, 1024);
load("MIDI");

setup_midi();

mod = controller_number("modulation");

start = 0.1;
dur = 0.1;
chan = 0;
pitch = 8.00;
vel = 0.8;

while (start < 3.0) {
	modvalue = irand(0,1);
	CONTROLLER(start, dur, chan, mod, modvalue);
	NOTE(start, dur, chan, pitch, vel);
	start += irand(0.01, 0.1);
}

