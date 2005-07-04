rtsetparams(44100, 2)
load("FILTSWEEP")

rtinput("../../snd/loocher.aiff")
inskip = 0
dur = DUR() - inskip

bypass = 0

amp = 1.1
env = maketable("line", 1000, 0,0, dur-.1,1, dur,0)

ringdur = 0.5
balance = 0
steepness = 2

lowcf = 90
highcf = 4000
narrowbw = -0.02
widebw = -0.90

cf = maketable("line", "nonorm", 2000, 0,lowcf, dur/3,highcf, dur,lowcf)
bw = maketable("line", "nonorm", 2000, 0,narrowbw, dur,widebw)

start = 0
FILTSWEEP(start, inskip, dur, amp * env, ringdur, steepness, balance,
	inchan=0, pan=1, bypass, cf, bw)

highcf = 5000
cf = maketable("line", "nonorm", 2000, 0,highcf, dur/3,lowcf, dur,highcf)
bw = maketable("line", "nonorm", 2000, 0,widebw, dur,narrowbw)

start = 0.1
FILTSWEEP(start, inskip, dur, amp * env, ringdur, steepness, balance,
	inchan=0, pan=0, bypass, cf, bw)

