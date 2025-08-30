// Test score for MIDI function calls and instruments

set_option("midi_outdevice = Internal MIDI Bus 1");
set_option("buffer_count=1");
print_on(6);
sample_rate = 48000;
buffer_size = 2048;
rtsetparams(sample_rate, 2, buffer_size);
load("MIDI");
load("WAVETABLE");

setup_midi();

dur = 1/4;
gain = 1;
chan = 0;
pitch = 8.07;
vel = 0.8;

wavetable = maketable("wave", 1000, 1, 0.3, 0.2)
wgain = 10000 * maketable("line", 1000, 0,1, 1,1, 2, 0)

start = ((3 * buffer_size) + 2000) / sample_rate;
printf("Event at time %f\n", start);
WAVETABLE(start, dur, wgain, cpspch(pitch), pan=1, wavetable)
//NOTE(start, dur, chan, pitch+0.06, vel);

start = ((23 * buffer_size) + 2000) / sample_rate;
printf("Event at time %f\n", start);
WAVETABLE(start, dur, wgain, cpspch(pitch), pan=1, wavetable)
NOTE(start, dur, chan, pitch+0.06, vel);

start = ((43 * buffer_size) + 2000) / sample_rate;
printf("Event at time %f\n", start);
WAVETABLE(start, dur, wgain, cpspch(pitch), pan=1, wavetable)
NOTE(start, dur, chan, pitch+0.06, vel);
