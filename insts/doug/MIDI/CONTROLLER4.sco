// Test score for CONTROLLER instrument cancelling

set_option("midi_outdevice = Internal MIDI: Bus 1");
rtsetparams(48000, 2, 1024);
load("MIDI");

setup_midi();

// We will use a table curve to generate a smooth controller change

expr = controller_number("expression");
exprvolume = maketable("line", "nonorm", 1000, 0, 0, 1, 1);

start = 0.1;
dur = 3;
chan = 0;
pitch = 8.00;
vel = 0.8;

PROGRAM(0, 0, 1, chan, 23);	// load accordian

// cancelling the controller before it is active should produce warning and be
// a no-op

CONTROLLER(start, 0.1, chan, expr, -1);

CONTROLLER(start+2, dur, chan, expr, exprvolume);
NOTE(start+2, dur, chan, pitch, vel);

// cancelling a controller now that it is active works

start = 6;

CONTROLLER(start, dur, chan, expr, exprvolume);
NOTE(start, dur, chan, pitch, vel);


CONTROLLER(start+1, 0.1, chan, expr, -1);
