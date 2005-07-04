rtsetparams(44100, 2)
load("FILTSWEEP")
load("JDELAY")

bus_config("FILTSWEEP", "in 0", "aux 0 out")
bus_config("JDELAY", "aux 0 in", "out 0", "out 1")

rtinput("../../snd/input.wav")
inskip = 0

masteramp = 3.0

/* --------------------------------------------------------------- sweep1 --- */
start = 0

amp = 2.5 * masteramp
balance = 0
sharpness = 3
ringdur = .2
bypass = 0

dur = DUR()
env = maketable("curve", 1000, 0,0,2, dur-.1,1,0, dur,0)

cf = maketable("line", "nonorm", 1000, 0,0, 1,1400, 2,0)
bw = maketable("line", "nonorm", 1000, 0,-.008, 1,-.7)

FILTSWEEP(start, inskip, dur, amp * env, ringdur, sharpness, balance,
	inchan=0, pan=0, bypass, cf, bw)

/* --------------------------------------------------------------- sweep2 --- */
start = dur + 2

amp = 0.05 * masteramp
cf = maketable("line", "nonorm", 1000, 0,0, 1,5000, 2,0)
bw = maketable("line", "nonorm", 1000, 0,-.01, 1,-.08)

FILTSWEEP(start, inskip, dur, amp * env, ringdur, sharpness, balance,
	inchan=0, pan=0, bypass, cf, bw)

/* ---------------------------------------------------------------- delay --- */
amp = maketable("line", 1000, 0,1, 9,1, 10,0)
regen = 0.88
ringdur = 12

dur = start + dur
wetdry = 1

deltime = 0.2
JDELAY(st=0, insk=0, dur, amp, deltime, regen, ringdur, cutoff=0, 
       wetdry, inchan=0, pan=1, 1)
deltime = 0.3
JDELAY(st, insk, dur, amp, deltime, regen, ringdur, cutoff=0, 
       wetdry, inchan=0, pan=0, 1)

