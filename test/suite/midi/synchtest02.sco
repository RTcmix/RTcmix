// Test score for MIDI function calls and instruments

set_option("midi_outdevice = Internal MIDI Bus 2");
set_option("buffer_count=1");
print_on(6);
rtsetparams(48000, 2, 2048);
load("MIDI");
load("WAVETABLE");

setup_midi();

dur = 1.0;
gain = 1;
chan = 0;
pitch = 8.07;
vel = 0.8;
incr = 0.01;

wavetable = maketable("wave", 1000, 1, 0.3, 0.2)
wgain = 10000 * maketable("line", 1000, 0,1, 10,1, 12, 0)

start = 1.73521;

WAVETABLE(start, dur, wgain, cpspch(pitch), pan=1, wavetable)
NOTE(start, dur, chan, pitch+0.06, vel);

WAVETABLE(5, 0, 0, cpspch(pitch), pan=1, wavetable)
