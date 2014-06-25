rtsetparams(44100, 2)
load("WAVETABLE")
load("FOLLOWGATE")

source_listen = 0  /* set to 1 to hear carrier and modulator separately */

dur = 20

/* play carrier to bus 0 */
bus_config("WAVETABLE", "aux 0 out")
makegen(2, 10, 1000, 1, .5, .2)
amp = 15000
WAVETABLE(0, dur, amp, freq = 440)
WAVETABLE(0, dur, amp, freq * 1.02)

/* play modulator to bus 1 */
bus_config("WAVETABLE", "aux 1 out")
setline(0,0, 1,1, 19,1, 20,0)
reset(20000)
incr = base_incr = 0.15
notedur = base_incr * 0.3
freq = 1000
for (st = 0; st < dur; st = st + incr) {
   amp = irand(5000, 30000)
   WAVETABLE(st, notedur, amp, freq)
   incr = base_incr * irand(0.5, 2)
}
reset(1000)

/* apply modulator's amp envelope to carrier */
bus_config("FOLLOWGATE", "aux 0-1 in", "out 0-1")
setline(0,0, 1,1, 10,1, 20,0)
caramp = 2.0
modamp = 5.0
winlen = 100      /* number of samples for power gauge to average */
smooth = 0.0      /* how much to smooth the power gauge curve */
attack = 0.002
release = 0.02
pctleft = 0.5
thresh = 0.5
range = 0.005
makegen(2, 18, 100, 0,thresh, 1,thresh)
makegen(3, 18, 1000, 0,range, 1,range)
if (source_listen) {
   bus_config("MIX", "aux 0-1 in", "out 0-1")
   MIX(0, 0, dur, 1, 0, 1)
}
else
   FOLLOWGATE(0, inskip = 0, dur, caramp, modamp, winlen, smooth, attack,
                                                         release, pctleft)

