// Test score for CONTROLLER instrument

set_option("midi_outdevice = Internal MIDI: Bus 2");
rtsetparams(48000, 2, 1024);
load("MIDI");

setup_midi();

// We will use a table curve to generate a smooth pitch bend change

bendcurve = maketable("line", "nonorm", 1000, 0, 0, 1, 1, 2, -1, 3, 0);

start = 0.1;
dur = 5;
chan = 0;
pitch = 8.00;
vel = 0.8;

PROGRAM(0, 0, 1, chan, 23);	// load accordian

NOTE(start, dur, chan, pitch, vel);
PITCHBEND(start, dur, chan, bendcurve);

