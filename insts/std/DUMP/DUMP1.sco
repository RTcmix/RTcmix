rtsetparams(44100, 1)
load("DUMP")

dur = 1
amp = maketable("line", 1000, 0,0, 1,1, 2,0)
//dumptable(amp)

amp = makefilter(amp, "fitrange", -100, 300)

reset(100)

DUMP(0, dur, amp)

