rtsetparams(44100, 2)
load("DUST")
load("FILTERBANK")

bus_config("DUST", "aux 0 out")
bus_config("FILTERBANK", "aux 0 in", "out 0-1")

amp = 14000
famp = 30
imprange = 0
ringdur = 1
inchan = 0
density1 = maketable("expbrk", "nonorm", 100, 3, 100, 0.5)
DUST(0, dur=18, amp, density1, imprange)
FILTERBANK(0, 0, dur=25, famp, ringdur, inchan, pan=0.7, cf=470, bw=0.0003, g=1)

bus_config("DUST", "aux 2-3 out")
bus_config("FILTERBANK", "aux 2-3 in", "out 0-1")

density2 = maketable("expbrk", "nonorm", 100, 0.5, 100, 5)
DUST(2, dur=20, amp, density2, imprange)
FILTERBANK(0, 0, dur=25, famp, ringdur, inchan, pan=0.3, cf=700, bw=0.0003, g=1)

