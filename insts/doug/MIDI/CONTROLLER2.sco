// Test score for CONTROLLER instrument

set_option("midi_outdevice = Internal MIDI Bus 2");
rtsetparams(48000, 2, 1024);
load("MIDI");

setup_midi();

// We will use a table curve to generate a smooth controller change

expr = controller_number("expression");
exprvolume = maketable("line", "nonorm", 1000, 0, 1, 1, 0.1, 2, 1);

start = 0.1;
dur = 5;
chan = 0;
pitch = 8.00;
vel = 0.8;

PROGRAM(0, 0, 1, chan, 23);	// load accordian (if we are talking to a GM synth)

CONTROLLER(start, dur, chan, expr, exprvolume);
NOTE(start, dur, chan, pitch, vel);

// CANCEL THIS CONTROLLER
CONTROLLER(2, 0.1, chan, expr, -1);

