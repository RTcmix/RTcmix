rtsetparams(44100, 2)
load("REVMIX")

rtinput("../../snd/input.wav")

dur = DUR()

REVMIX(start=0, inskip=dur, dur, amp=1)

