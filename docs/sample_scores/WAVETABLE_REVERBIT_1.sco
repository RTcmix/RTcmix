/* This shows how to add global reverb to any score. */
rtsetparams(44100, 2)
load("WAVETABLE")
load("REVERBIT")

/* WAVETABLE output enters aux 0-1 buses; REVERBIT reads from those,
   and sends output to sound card.
*/
bus_config("WAVETABLE", "aux 0-1 out")
bus_config("REVERBIT", "aux 0-1 in", "out 0-1")

/* Controls duration of WAVETABLE loop and length of REVERBIT call. */
totdur = 6

reset(2000)   /* otherwise, the short WAVETABLE notes click */

/*----------------------------------------------------------------------------*/
setline(0,0, 1,1, 5,0)
makegen(2, 10, 10000, 1,.5,.3,.1)

dur = 1
amp = 10000
freq = 9.00
pctleft = 1

srand(3284)

dur = .08
for (st = 0; st < totdur; st = st + .3)
   WAVETABLE(st, dur, amp, freq, random())

freq = 7.10
for (st = 0; st < totdur; st = st + .45)
   WAVETABLE(st, dur, amp, freq, random())

/*----------------------------------------------------------------------------*/
amp = 1
revtime = 1.5
revpct = .2
rtchandel = .15
cf = 200

setline(0,1, 1,1)  /* override WAVETABLE's envelope */

/* inskip MUST be zero when reading from aux bus */
REVERBIT(st=0, insk=0, totdur, amp, revtime, revpct, rtchandel, cf)

