rtsetparams(44100, 2)
load("WAVETABLE")
load("REVERBIT")

bus_config("WAVETABLE", "aux 0-1 out")
bus_config("REVERBIT", "aux 0-1 in", "out 0-1")

totdur = 10

reset(5000)

/*----------------------------------------------------------------------------*/
setline(0,0, 1,1, 5,0)
makegen(2, 10, 10000, 1,.5,.3,.1)

amp = 10000
freq = 9.00
pctleft = 1

srand(3284)

dur = .08
for (st = 0; st < totdur; st = st + .3)
   WAVETABLE(st, dur, amp, freq, random())

bus_config("WAVETABLE", "aux 2-3 out")

freq = 7.10
for (st = 0; st < totdur; st = st + .45)
   WAVETABLE(st, dur, amp, freq, random())


/*----------------------------------------------------------------------------*/
amp = 1
revtime = 1.5
revpct = .2
rtchandel = .15
cf = 200

setline(0,1, 1,1)
REVERBIT(st=0, insk=0, totdur, amp, revtime, revpct, rtchandel, cf)


/*----------------------------------------------------------------------------*/
bus_config("REVERBIT", "aux 2-3 in", "out 0-1")

amp = .5
revtime = 0.5
revpct = .8
rtchandel = .45 / 2
cf = 0

setline(0,1, 1,1)
REVERBIT(st=0, insk=0, totdur, amp, revtime, revpct, rtchandel, cf)

