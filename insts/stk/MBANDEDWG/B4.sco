rtsetparams(44100, 2)
load("MBANDEDWG")

amp = maketable("line", 1000, 0,1, 4,1, 5,0)
freq = maketable("line", "nonorm", 1000, 0,100, 1,250)
bp = maketable("line", 1000, 0,0.668, 1,0.2, 2,0.9)
vel = makeLFO("sine", 0.1, 0.0, 1.0)
pan = makeLFO("sine", 0.5, 0.0, 1.0)
MBANDEDWG(0, 10.0, amp*5000, freq, 0.3, 0, 0.5, 3, 0.3, 0.2, bp, pan, vel)
