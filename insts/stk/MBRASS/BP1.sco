rtsetparams(44100, 2)
load("./libMBRASS.so")

freq = maketable("line", "nonorm", 1000, 0,201, 1,314)
pan = makeLFO("tri", 0.5, 0.0, 1.0)
MBRASS(0, 3.5, 15000, freq, 200, 279.0, 0.3, pan)

bamp = maketable("line", 1000, 0,0, 1,1, 2,0)
slide = maketable("line", "nonorm", 1000, 0, 100, 1, 400, 3, 200)
lipfilt = makeLFO("sine", 3.5, 250, 400)
MBRASS(4, 3.5, 20000, 249.0, slide, lipfilt, 0.3, 0.5, bamp)
