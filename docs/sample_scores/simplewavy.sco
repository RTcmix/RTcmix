// demonstration of new experimental table "drawing" feature.  Move the
// mouse in the window that appears to redraw (blindly) the waveform
// as the note plays.  -JGG

rtsetparams(44100, 2, 256)
load("WAVY")

dur = 60
amp = 10000
pitch = 8.00

wavet = maketable("wave", 1000, "sine")

index = makeconnection("mouse", "x", min=0, max=1, dflt=0, lag=80, "index")
value = makeconnection("mouse", "y", min=-1, max=1, dflt=0, lag=80, "value")
wavet = modtable(wavet, "draw", index, value, .1)

WAVY(start=0, dur, amp, pitch, 0, phase_offset=0, wavet, 0, "a", pan=.5)

