rtsetparams(44100, 2)
load("WAVETABLE")
load("FOLLOWER")

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
bus_config("FOLLOWER", "aux 0-1 in", "out 0-1")
setline(0,0, 1,1, 10,1, 20,0)
caramp = 1.0
modamp = 4.0
winlen = 10       /* number of samples for power gauge to average */
smooth = 0.5      /* how much to smooth the power gauge curve */
pctleft = 0.5
if (source_listen) {
   bus_config("MIX", "aux 0-1 in", "out 0-1")
   MIX(0, 0, dur, 1, 0, 1)
}
else
   FOLLOWER(0, inskip = 0, dur, caramp, modamp, winlen, smooth, pctleft)

