rtsetparams(44100, 2)
load("REV")

rtinput("foo.aif")

outskip = 0
inskip = 0
dur = DUR()
amp = 1.0
rvbtype = 3
rvbtime = 0.3
rvbpct = 0.3
inchan = 0

setline(0,0, 1,1, dur-1,1, dur,0)

REV(outskip, inskip, dur, amp, rvbtype, rvbtime, rvbpct, inchan)
