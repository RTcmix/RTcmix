rtsetparams(44100, 2)
load("WAVETABLE")

dur = 15
amp = 6000
makegen(1, 4, 1000, 0,0,1, 1,1,0, 3,1,-1, 4,0)

freq = 30
makegen(2, 20, 4000, 0, 1, -1, 1)
WAVETABLE(0, dur, amp, freq, 0)

makegen(2, 20, 3000, 0, 2, -1, 1)
WAVETABLE(0, dur, amp, freq, 1)
