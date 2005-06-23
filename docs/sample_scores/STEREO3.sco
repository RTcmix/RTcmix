rtsetparams(44100, 2)
load("STEREO")

reset(10000)
makegen(1, 25, 10000, 1)

rtinput("../../snd/huhh.wav")

totdur = 10
incr = .02
dur = .25
amp = 1.0
min_inskip = 0.7
max_inskip = DUR(0) - dur
inskip_diff = max_inskip - min_inskip

for (st = 0; st < totdur; st = st + incr) {
   inskip = min_inskip + (inskip_diff * random())
   pctleft = random()
   STEREO(st, inskip, dur, amp, pctleft)
}

