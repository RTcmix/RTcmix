rtsetparams(44100, 2)
load("FILTSWEEP")
load("JDELAY")

bus_config("FILTSWEEP", "in 0", "aux 0 out")
bus_config("JDELAY", "aux 0 in", "out 0", "out 1")

rtinput("/snd/Public_Sounds/vccm_old/hitgong.snd")
inskip = 0

masteramp = 2.0

/* --------------------------------------------------------------- sweep1 --- */
amp = 1.0 * masteramp
balance = 1
sharpness = 3
ringdur = .2

dur = 8
setline(0,0, dur-.1,1, dur,0)

makegen(2, 18, 2000,   0, 0,  1, 1400,  2, 0)    /* center freq. curve */
makegen(3, 18, 2000,   0, -.008,  1, -.7)        /* bandwidth curve */

start = 0
FILTSWEEP(start, inskip, dur, amp, ringdur, sharpness, balance)

/* --------------------------------------------------------------- sweep2 --- */
amp = 1.0 * masteramp
makegen(2, 18, 2000,   0, 0,  1, 5000,  2, 0)
makegen(3, 18, 2000,   0, -.008,  1, -.08)

start = 8.5
FILTSWEEP(start, inskip, dur, amp, ringdur, sharpness, balance)

/* ---------------------------------------------------------------- delay --- */
amp = 1.0
regen = 0.88
ringdur = 12

setline(0,1, 9,1, 10,0)

dur = dur * 2.2

deltime = 0.2
JDELAY(st=0, insk=0, dur, amp, deltime, regen, ringdur, cutoff=0, 
       wetdry=1, inchan=0, pctleft=1, 1)
deltime = 0.3
JDELAY(st, insk, dur, amp, deltime, regen, ringdur, cutoff=0, 
       wetdry=1, inchan=0, pctleft=0, 1)

