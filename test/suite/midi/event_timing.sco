// NOTE(start, duration, MIDI chan, pitch (oct pt pc), velocity (0-1.0)

slice = 1024;
sr = 48000;
set_option("midi_outdevice = Internal MIDI Bus 2");
rtsetparams(sr, 2, slice);
load("MIDI");

setup_midi();

chan = 0;
pitch = 7.00;
vel = 0.5;

NOTE(500/sr, 400/sr, chan, pitch, vel);
NOTE(700/sr, 300/sr, chan, pitch+0.02, vel);
NOTE(1000/sr, 1.0, chan, pitch+0.04, vel);

