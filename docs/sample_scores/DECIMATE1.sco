rtsetparams(44100, 2)
load("DECIMATE")

rtinput("../../snd/nucular.wav")
rtinput("../../snd/huhh.wav")

inchan = 0
dur = DUR()

bits = 2
preamp = 2
postamp = maketable("line", 1000, 0,0, 5,1, 9,1, 10,0)
cutoff = maketable("line", "nonorm", 1000, 0,1, 1,10000, 2,800)
pan = maketable("line", 100, 0,0, 1,1)

DECIMATE(0, 0, dur, preamp, postamp, bits, cutoff, inchan, pan)

