rtsetparams(44100, 2)
load("WAVETABLE")
load("REVERBIT")

totdur = 10

control_rate(5000)

//--- synth --------------------------------------------------------------------
amp = 10000 * maketable("line", 1000, 0,0, 1,1, 5,0)
pitch = 9.00
wavet = maketable("wave", 10000, "saw9")
srand(3284)

dur = 0.07
incr1 = 0.3
incr2 = 0.45

bus_config("WAVETABLE", "aux 0-1 out")
for (st = 0; st < totdur; st += incr1)
   WAVETABLE(st, dur, amp, pitch, random(), wavet)

pitch = 7.10
bus_config("WAVETABLE", "aux 2-3 out")
for (st = 0; st < totdur; st += incr2)
   WAVETABLE(st, dur, amp, pitch, random(), wavet)

//--- reverb 1 -----------------------------------------------------------------
bus_config("REVERBIT", "aux 0-1 in", "out 0-1")
amp = 1.5
revtime = 1.5
revpct = 0.2
rtchandel = incr1 / 2
cf = 200

REVERBIT(st=0, insk=0, totdur, amp, revtime, revpct, rtchandel, cf)

//--- reverb 2 -----------------------------------------------------------------
bus_config("REVERBIT", "aux 2-3 in", "out 0-1")

amp = 1.0
revtime = 0.5
revpct = 0.8
rtchandel = incr2 / 2
cf = 0

REVERBIT(st=0, insk=0, totdur, amp, revtime, revpct, rtchandel, cf)

