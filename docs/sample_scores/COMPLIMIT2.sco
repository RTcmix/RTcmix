// This is a terrible example, but it'll have to do.  -JGG

rtsetparams(44100, 2)
load("COMPLIMIT")

rtinput("../../snd/loocher.aiff")
inskip = 0
dur = DUR()

ingain = 18     // that dog's too loud!
outgain = 0
attack = 0.001
release = 0.02
threshold = -20
ratio = 4
lookahead = attack
windowlen = 128
detect_type = 0
bypass = 1

COMPLIMIT(0, inskip, dur, ingain, outgain, attack, release, threshold,
          ratio, lookahead, windowlen, detect_type, bypass, inchan=0, pan=.5)

