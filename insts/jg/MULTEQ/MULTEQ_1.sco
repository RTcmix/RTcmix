rtsetparams(44100, 2)
load("MULTEQ")

rtinput("mystereofile.wav")
inskip = 0
dur = DUR()
amp = 1
bypass = 0

type1 = "lowshelf"
freq1 = maketable("line", "nonorm", 100, 0,100, 1,100, 3,1000)
Q1 = 2
gain1 = maketable("line", "nonorm", 100, 0,6, 1,6, 3,-6)
bypass1 = 0

type2 = "highshelf"
freq2 = 2000
Q2 = 1
gain2 = maketable("line", "nonorm", 100, 0,-12, 1,9, 2,0)
bypass2 = 0

MULTEQ(0, inskip, dur, amp, bypass,
   type1, freq1, Q1, gain1, bypass1,
   type2, freq2, Q2, gain2, bypass2)

