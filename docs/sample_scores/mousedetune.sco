rtsetparams(44100, 2, 128)
load("WAVETABLE")
orig = 1; record = 2; play = 3

// Rearrange order of these three statements; the one in effect is the last one.
// Start by playing the original and moving your mouse in the mouse window to
// change the amount of detuning and pan.  Then try recording your detune
// motions and playing them back back changing these modes.

mode = play
mode = record
mode = orig

dur = 30
freq = 200

amp = 6000
env = maketable("line", 5000, 0,0, 1,1, dur-1,1, dur,0)

// sawtooth waveform
wavetable = maketable("wave", 6000, 1, 1/2, 1/3, 1/4, 1/5, 1/6, 1/7, 1/8, 1/9)

if (mode != play)
   detune = makeconnection("mouse", "y",
                           min = 0, max = 6, dflt = 0, lag = 50,
                           "detune", "Hz")
if (mode == record)
	detune = makedatafile(detune, "mousedetune.detune", 200)
else if (mode == play) {
   starttime = 0.0
   timefact = 1.0   // < 1: play data faster; > 1: play data slower
   detune = makeconnection("datafile", "mousedetune.detune", lag = 50,
                           starttime, timefact)
}

pan = makeconnection("mouse", "x", min=1, max=0, dflt=.5, lag=80, "pan")

WAVETABLE(0, dur, amp * env, freq, pan, wavetable)
WAVETABLE(0, dur, amp * env, freq + detune, pan, wavetable)


// -JGG, 3/17/05
