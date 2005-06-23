rtsetparams(44100, 2)
load("./libMCLAR.so")

amp = maketable("line", 1000, 0,0, 1,1, 2,1, 3,0)
freq = maketable("line", "nonorm", 1000, 0, 349, 1, 207)
MCLAR(0, 3.5, amp*20000.0, freq, 0.2, 0.7, 0.5)

bamp = maketable("line", 1000, 0,1, 2,1, 3,0)
noise = makeLFO("sine", 7.0, 0.0, 1.0)
reedstiff = maketable("line", 1000, 0,0, 1,1, 2,0)
pan = maketable("line", 1000, 0,1, 1,0, 1.5,1, 2,0)
MCLAR(4, 3.5, 15000.0, 149.0, noise, 0.7, reedstiff, pan, bamp)
