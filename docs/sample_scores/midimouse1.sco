// For OS X, the MIDI input device name consists of a name, a colon and space,
// and then the port name.  You can see these names in the Audio MIDI Setup
// program.  (Dbl-click the interface icon, and look at the "Device Name"
// field.  Then click on the Ports tab, and look at the port names.)

set_option("midi_indevice = FastLane USB: Port A")

bufsize = 128	// must have low bufsize, or else it won't work smoothly
rtsetparams(44100, 2, bufsize)
load("WAVETABLE")

dur = 60

lag = 60
dB = makeconnection("midi", min=0, max=90, dflt=60, lag, chan=1, "cntl", "mod")
amp = makeconverter(dB, "ampdb")

lag = 10	// If lag is too high, the freqs will not be accurate.
notenum = makeconnection("midi", min=0, max=127, dflt=60, lag, chan=1,
                         "noteon", "pitch")
freq = makeconverter(notenum, "pchmidi")

pan = makeconnection("mouse", "x", min=1, max=0, dflt=.5, lag=50, "pan")
//pan = 0.5

wavt = maketable("wave", 4000, 1, .3, .2, .1)

WAVETABLE(0, dur, amp, freq, pan, wavt)


// -JGG, 1/28/05

