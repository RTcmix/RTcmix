rtsetparams(44100, 2)
load("STEREO")

reset(10000)
xsetline(0,0, 1,1, 5,1, 6,0)
makegen(1, 25, 10000, 1)

rtinput("/snd/Public_Sounds/vccm_old/ah.snd")

totdur = 10
incr = .02
dur = .15
amp = 3.0
min_inskip = 0.7
max_inskip = 4.5
inskip_diff = max_inskip - min_inskip

for (st = 0; st < totdur; st = st + incr) {
   inskip = min_inskip + (inskip_diff * random())
   pctleft = random()
   STEREO(st, inskip, dur, amp, pctleft)
}

