rtsetparams(44100, 2)
load("WAVETABLE")
load("FILTSWEEP")

bus_config("WAVETABLE", "aux 0 out")
bus_config("FILTSWEEP", "aux 0 in", "out 0-1")

/*----------------------------------------------------------------------------*/
setline(0,0, 1,1, 9,1, 10,0)
makegen(2, 10, 10000, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1)

start = 0
totdur = 10
dur = .06
incr = .18
amp = 10000

for (st = start; st < totdur; st = st + incr) {
   WAVETABLE(st, dur, amp, 6.00)
   WAVETABLE(st, dur, amp, 7.00)
   WAVETABLE(st, dur, amp, 7.07)
}

/*----------------------------------------------------------------------------*/
amp = 1.0

balance = 0
sharpness = 2.2
ringdur = .2

lowcf = 300
highcf = 4000

setline(0,0, 1,1, 4,1, 5,0)
makegen(2,18,2000, 0,lowcf, 1,highcf, 2,lowcf, 2.2,lowcf, 3,highcf, 4,lowcf)
makegen(3,18,2000, 0,-.16, 1,-.16)

reset(2000)

FILTSWEEP(start, inskip=0, totdur, amp, ringdur, sharpness, balance)

