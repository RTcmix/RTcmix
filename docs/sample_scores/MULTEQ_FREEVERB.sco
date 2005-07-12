rtsetparams(44100, 2)
load("MULTEQ")
load("FREEVERB")

bus_config("MULTEQ", "in 0", "aux 0 out")
bus_config("FREEVERB", "aux 0 in", "out 0-1")

rtinput("../../snd/loocher.aiff")
inskip = 0
dur = DUR()

amp = 0.5
bypass = 0

type1 = "lowshelf"
freq1 = maketable("line", "nonorm", 100, 0,400, 1,400, 3,900)
Q1 = 2
gain1 = maketable("line", "nonorm", 100, 0,12, 1,0, 2,-12)
bypass1 = 0

type2 = "highshelf"
freq2 = 2000
Q2 = 1
gain2 = maketable("line", "nonorm", 100, 0,-12, 2,6, 3,0)
bypass2 = 0

MULTEQ(0, inskip, dur, amp, bypass,
   type1, freq1, Q1, gain1, bypass1,
   type2, freq2, Q2, gain2, bypass2)


roomsz = 0.7
predel = 0.01
ringdur = 2.0
damp = 0
wet = 30
FREEVERB(0, 0, dur, 1, roomsz, predel, ringdur, damp, 100-wet, wet, 100)

