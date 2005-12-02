// This is set up to use the OS X P5 glove driver by Tim Kreger
// <http://www.audiomulch.com/simulus/p5glove/P5osc_osx_20040428.tar.gz>.
// This formats the 11 pieces of data as a single OSC message using
// the "/p5glove_data" path.  The last 3 items are the X, Y and Z
// positions of the glove.                   -John Gibson, 12/2/05

rtsetparams(44100, 2, 256)
load("WAVETABLE")

dur = 20

// default port for the p5osc driver
set_option("osc_inport = 47110")

// Note that <outmin> and <outmax> are in reversed order for amp
// and freq, so that we can have a high amp represented by a high
// glove position, and a low frequency by a left position.

// up and down for amp (in dB)
amp = makeconnection("osc", "/p5glove_data", index=9,
         inmin=-500, inmax=500, outmin=40, outmax=90, dflt=0, lag=70)
amp = makeconverter(amp, "ampdb")

// this amp modulation makes it easier to hear panning
lwave = maketable("line", "nonorm", 1000, 0,-1, 5,1, 20,1, 25,-1, 100,-1)
lfo = makeLFO(lwave, lfreq=6, min=0, max=1)

// left-to-right for freq (low -> high)
freq = makeconnection("osc", "/p5glove_data", index=8,
         inmin=-500, inmax=500, outmin=1500, outmax=100, dflt=500, lag=70)

// forward/backward for pan
pan = makeconnection("osc", "/p5glove_data", index=10,
         inmin=-500, inmax=500, outmin=0, outmax=1, dflt=0.5, lag=70)

wavet = maketable("wave", 4000, "tri20")

WAVETABLE(0, dur, amp * lfo, freq, pan, wavet)

