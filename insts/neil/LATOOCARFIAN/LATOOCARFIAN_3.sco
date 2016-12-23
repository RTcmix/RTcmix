rtsetparams(44100, 2)
load("./libLATOOCARFIAN.so")

a = 2.176
b = makeLFO("square", 1, 2.5575, 2.926499)
c = 0.5
d = 0.5

dur = 8
amp = makeLFO("tri", 1/3, 3000, 10000)
LATOOCARFIAN(0, dur, amp, a, b, c, d, seedx=0.5, seedy=0.75, pan=0)
LATOOCARFIAN(0, dur, amp, a, b, c, d, seedx=0.75, seedy=0.5, pan=1)
