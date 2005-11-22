// NB: default audio device (in .rtcmixrc) must work with <numchans> chans.
set_option("print = off")
if (n_arg() != 1) {
	printf("*** Usage: CMIX numchans < test-n-channel-blip.sco\n")
	exit(1)
}
numchans = i_arg(0)
printf("Playing to %d output channels\n", numchans)
rtsetparams(44100, numchans)
load("WAVETABLE")

count = 5
freq = 440
wavet = maketable("wave", 1000, "sine")
amp = maketable("line", 1000, 0,0, 1,1, 19,1, 20,0) * 10000
dur = 0.25

st = 0
for (i = 0; i < count; i += 1) {
	for (c = 0; c < numchans; c += 1) {
		bus_config("WAVETABLE", "out " + c)
		WAVETABLE(st, dur, amp, freq, 0, wavet)
		st += dur + 0.25
	}
}

