/* This just takes Brad's ../sample_scos_1.0/STRUM6.sco and sends it
   through a global reverb.    -JGG
*/
rtsetparams(44100, 2)
load("STRUM")
load("REVERBIT")

bus_config("START1", "aux 0 out")
bus_config("BEND1", "aux 0 out")
bus_config("FRET1", "aux 0 out")
bus_config("REVERBIT", "aux 0 in", "out 0-1")

/*----------------------------------------------------------------------------*/
makegen(1, 24, 1000, 0,1,1,1)
START1(0, 2, 6.08, 1, 1, 10, 0.05, 7.00, 0, 1, 10000, 2)
makegen(10, 24, 1000, 0, 0, 1, 1, 2, 0)
BEND1(2, 4, 6.08, 7.00, 10, 1, 1, 10, 0.05, 7.00, 0, 1, 10000, 100)
FRET1(6, .2, 6.10, 1, 1, 10, 0.05, 7.00, 0, 1, 10000)
FRET1(6.2, .1, 7.00, 1, 1, 10, 0.05, 7.00, 0, 1, 10000)
FRET1(6.3, .1, 7.02, 1, 1, 10, 0.05, 7.00, 0, 1, 10000)
FRET1(6.4, .1, 7.00, 1, 1, 10, 0.05, 7.00, 0, 1, 10000)
FRET1(6.5, 1, 6.10, 1, 1, 10, 0.5, 7.00, 0, 1, 10000)
FRET1(7.5, 1, 6.10, 1, 1, 10, 0.5, 7.07, 0, 1, 10000)
FRET1(8.5, 1, 6.10, 1, 1, 10, 0.5, 7.09, 0, 1, 10000)
FRET1(9.5, 2, 6.10, 1, 1, 10, 0.5, 6.01, 0, 1, 10000)
FRET1(11.5, 2, 6.10, 1, 1, 10, 0.5, 8.01, 0, 1, 10000)

/*----------------------------------------------------------------------------*/
amp = 1.0
revtime = 2.0
revpct = .7
rtchandel = .16
cf = 800
dur = 13.5

setline(0,1, 1,1)  /* override makegen 1 above */

/* inskip MUST be zero when reading from aux bus */
REVERBIT(st=0, insk=0, dur, amp, revtime, revpct, rtchandel, cf)

