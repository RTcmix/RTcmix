rtsetparams(44100, 2)
load("VOCODESYNTH")

rtinput("../../../snd/input.wav")
inskip = 0.0
dur = DUR()
amp = 4.0
numbands = 22
lowcf = 300
interval = 0.025
cartransp = 0.00
bw = 0.06
winlen = 0.0001
smooth = 0.98
thresh = 0.0001
atktime = 0.001
reltime = 0.01
hipassmod = 0.0
hipasscf = 2000

wavetable = maketable("wave", 10000, "sine")

scale1 = .5
scale2 = 1
scaletab = maketable("curve", "nonorm", 100, 0,scale1,1, 1,scale2)

env = maketable("line", 1000, 0,0, .1,1, dur-.1,1, dur,0)

spacemult = cpspch(interval) / cpspch(0.0)

VOCODESYNTH(0, inskip, dur, amp * env, numbands, lowcf, spacemult, cartransp,
	bw, winlen, smooth, thresh, atktime, reltime, hipassmod, hipasscf,
	inchan=0, pan=0.5, wavetable, scaletab)

