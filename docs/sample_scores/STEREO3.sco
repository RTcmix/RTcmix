rtsetparams(44100, 2)
load("STEREO")

// to make sure these very short notes are enveloped precisely
reset(5000)

env = maketable("window", 10000, "hanning")

rtinput("../../snd/huhh.wav")

totdur = 10
incr = 0.02
dur = 0.25
amp = 1.0 * env
min_inskip = 0.7
max_inskip = DUR() - dur
inskip_diff = max_inskip - min_inskip

for (st = 0; st < totdur; st += incr) {
   inskip = min_inskip + (inskip_diff * random())
   pan = random()
   STEREO(st, inskip, dur, amp, pan)
}

