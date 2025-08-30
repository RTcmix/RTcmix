// Test score for MIDI function calls and instruments

set_option("midi_outdevice = Internal MIDI Bus 2");
print_on(6);
rtsetparams(48000, 2, 32);
load("MIDI");

tempo(0, 156);

setup_midi();
set_option("send_midi_record_auto_start=1");

start = 0;
dur = 0.1;
chan = 0;
vel = 0.8;
prevtime = 0;

for (beat = 0; beat < 600; beat += 1) {
	time = tb(beat);
	printf("beat %d ==> time %f (delta = %f)\n", beat, time, time - prevtime);
	prevtime = time;
	NOTE(time, dur, chan, 9.00, vel);
}

