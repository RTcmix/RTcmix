/* A synthy sound, made by fm granular synthesis, passes through a flanger
   and then a reverb.
*/
rtsetparams(44100, 2);
load("JGRAN")
load("FLANGE")
load("REVERBIT")

bus_config("JGRAN", "aux 0-1 out")
bus_config("FLANGE", "aux 0-1 in", "aux 2-3 out")
bus_config("REVERBIT", "aux 2-3 in",  "out 0-1")

totdur = 25
masteramp = 1.0

/* ----------------------------------------------------------------- gran --- */
makegen(2, 25, 10000, 1)                    /* grain env: hanning window */
makegen(3, 10, 10000, 1)                    /* grain waveform: sine */
makegen(4, 18, 1000, 0,2, 1,2)              /* mod multiplier */
makegen(5, 18, 1000, 0,5, 1,5)              /* mod index */
makegen(6, 18, 1000, 0,300, 1,300)          /* grain freq min */
makegen(7, 18, 1000, 0,300, 1,300)          /* grain freq max */
makegen(8, 18, 1000, 0,20, 1,20)            /* grain speed min */
makegen(9, 18, 1000, 0,40, 1,40)            /* grain speed max */
makegen(10, 18, 1000, 0,65, 1,65)           /* grain intensity min (dB) */
makegen(11, 18, 1000, 0,80, 1,80)           /* grain intensity max */
makegen(12, 18, 1000, 0,1, 1,1)             /* grain density */
makegen(13, 18, 1000, 0,.5, 1,.5)           /* grain stereo loc */
makegen(14, 18, 1000, 0,1, 1,1)             /* grain stereo loc randomization */

JGRAN(start=0, totdur, amp=1, seed=1, type=1, ranphase=0)

/* --------------------------------------------------------------- flange --- */
resonance = 0.6
maxdelay = 0.015
moddepth = 100
modspeed = 0.2
wetdrymix = 0.5
flangetype = 0

makegen(2,10,20000, 1)

FLANGE(st=0, insk=0, totdur, amp=1, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan=0, pctleft=1)
maxdelay = maxdelay + .02
FLANGE(st=0, insk=0, totdur, amp=1, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan=1, pctleft=0)

/* --------------------------------------------------------------- reverb --- */
revtime = 1.0
revpct = .7
rtchandel = .1
cf = 1000

setline(0,0, 1,1, 2,1, 3,0)

REVERBIT(st=0, insk=0, totdur, masteramp, revtime, revpct, rtchandel, cf)




/* john gibson, 17-june-00 */
