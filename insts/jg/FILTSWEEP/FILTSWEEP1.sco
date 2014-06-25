rtsetparams(44100, 2)
load("FILTSWEEP")

rtinput("/snd/Public_Sounds/steeldrums.aiff")

inskip = 0
dur = DUR() - inskip

amp = 1.2
balance = 0
steepness = 2
ringdur = .5

lowcf = 0
highcf = 1600
startbw = -.01
stopbw = -1

setline(0,0, dur-.1,1, dur,0)

makegen(2,18,2000, 0,lowcf, dur/3,highcf, dur,lowcf)
makegen(3,18,2000, 0,startbw, dur,stopbw)

FILTSWEEP(start=0, inskip, dur, amp, ringdur, steepness, balance, 0,1)
FILTSWEEP(start=.1, inskip, dur, amp, ringdur, steepness, balance, 0,0)

