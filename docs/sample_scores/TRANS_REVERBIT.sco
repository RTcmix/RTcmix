rtsetparams(44100, 2)
load("TRANS")
load("REVERBIT")

bus_config("TRANS", "in 0", "aux 0-1 out")
bus_config("REVERBIT", "aux 0-1 in", "out 0-1")

totdur = 20

//----------------------------------------------------------------------
rtinput("../../snd/conga.snd")
inskip = 0
dur = DUR()

incr = 0.14
srand(3)

for (start = 0; start < totdur; start += incr) {
   pan = random()
   gain = irand(-50, 0)
   trans = irand(-0.02, 0.01)
   TRANS(start, inskip, dur, ampdb(gain), trans, 0, pan)
}

//----------------------------------------------------------------------
amp = 1.1
rvbtime = maketable("curve", "nonorm", 100, 0,0.3,3, 2,1,-4, 3,0)
rvbpct = maketable("line", 100, 0,0, 1,1, 4,0.5)
rtchandelay = 0.02
cutoff = 3000

totdur += dur
REVERBIT(0, 0, totdur, amp, rvbtime, rvbpct, rtchandelay, cutoff)

