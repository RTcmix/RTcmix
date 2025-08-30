// Test score for MIDI function calls and instruments

//set_option("midi_outdevice = Internal MIDI Bus 2");
set_option("midi_outdevice = Internal MIDI Bus 1");
set_option("buffer_count=1");
print_on(6);
sample_rate = 48000;
buffer_size = 2048;
rtsetparams(sample_rate, 2, buffer_size);
load("MIDI");
load("WAVETABLE");

setup_midi();

start = 0;
dur = 1/6;
gain = 1;
chan = 0;
pitch = 8.07;
vel = 0.8;


// make each event start at a different point in the buffer

incr = (buffer_size * 66/7)/sample_rate;
printf("incr = %f\n", incr);

wavetable = maketable("wave", 1000, 1, 0.3, 0.2)
wgain = 10000 * maketable("line", 1000, 0,1, 1,1, 2, 0)

for (n = 0; n < 5; ++n) {
	printf("Events at time %f\n", start);
	WAVETABLE(start, dur, wgain, cpspch(pitch), pan=1, wavetable)
	NOTE(start, dur, chan, pitch+0.06, vel);
	start += incr;
}

