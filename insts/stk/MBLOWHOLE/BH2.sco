rtsetparams(44100, 2)
load("./libMBLOWHOLE.so")

amp = maketable("line", 1000, 0,0, 1,1, 1.1,0)
ventstate = makeLFO("sine", 1, 0.0, 1.0)
MBLOWHOLE(0, 3.5, amp*20000.0, 414.0, 0.2, 0.7, 0.5, 0, ventstate)

breathamp = maketable("line", 1000, 0,0, 1,1, 2,1, 5,0)
freq = maketable("line", "nonorm", 1000, 0,300, 1, 500)
noiseamp = makeLFO("saw", 8.0, 1.0, 0.0)
pan = makeLFO("sine", 1.4, 0.0, 1.0)
MBLOWHOLE(4, 5.2, 19000.0, freq, noiseamp, 0.7, 0.2, 0, 1, pan, breathamp)

reedstiff = maketable("line", "nonorm", 1000, 0,0.6, 1,0.81, 2,0)
tonehole = makeLFO("sine", 1, 0.0, 1.0)
ventstate = makeLFO("sine", 3.5, 0.0, 1.0)
MBLOWHOLE(10, 3.5, 20000.0, 214.0, 0.2, 0.7, reedstiff, tonehole, ventstate)
