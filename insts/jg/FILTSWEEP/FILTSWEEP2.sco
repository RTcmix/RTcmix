rtsetparams(44100, 2)
load("FILTSWEEP")

rtinput("/snd/Public_Sounds/motorclip.snd")

inskip = 0
dur = DUR() - inskip

amp = .5
balance = 1
steepness = 2
bw = -.3
ringdur = .5

reset(5000)
setline(0,0, 1,1, dur-1,1, dur,0)

/* left chan */
makegen(2,18,2000, 0,20, dur,4000)            /* increasing cf */
makegen(3,18,2000, 0,bw,  dur,bw)             /* constant bw */
FILTSWEEP(start=0, inskip, dur, amp, ringdur, steepness, balance,
          inchan=0, pctleft=1)

/* right chan */
makegen(2,18,2000, 0,4000, dur,20)            /* decreasing cf */
makegen(3,18,2000, 0,bw,  dur,bw)             /* constant bw */
FILTSWEEP(start=.01, inskip, dur, amp, ringdur, steepness, balance,
          inchan=1, pctleft=0)

