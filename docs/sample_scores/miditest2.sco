// See miditest1.sco for more info.

set_option("midi_indevice = FastLane USB: Port A")

bufsize = 128
rtsetparams(44100, 2, bufsize)
load("WAVETABLE")

dur = 300

if (0) {
	freq = makeconnection("midi", min=100, max=2000, dflt=min, lag=50, chan=1,
	                      "cntl", 93)
	freq = makemonitor(freq, "display", "freq", "Hz", 2)
}
else {
	note = makeconnection("midi", min=0, max=127, dflt=60, lag=10, chan=1,
	                      "noteon", "pitch")
	note = makemonitor(note, "display", "note", "MIDI", 0)
	freq = makeconverter(note, "pchmidi")
}

pan = makeconnection("midi", min=1, max=0, dflt=.5, lag=50, chan=1, "cntl", 72)
pan = makemonitor(pan, "display", "pan", 2)

amp = makeconnection("midi", min=0, max=20000, dflt=8000, lag=50, chan=1,
                     "cntl", 1)
amp = makemonitor(amp, "display", "amp")

wavt = maketable("wave", 4000, 1, .3, .2, .1)

WAVETABLE(0, dur, amp, freq, pan, wavt)


// -JGG, 2/10/05

