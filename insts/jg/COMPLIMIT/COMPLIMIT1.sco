rtsetparams(44100, 1)
xload("COMPLIMIT")

rtinput("/tmp/jumpyextract.wav")

outskip = 0
inskip = 10
dur = .5
ingain = 1.0
outgain = 1.0
attack = 0.001
release = 0.001
threshold = 22000
ratio = 100000.0
windowlen = 64
type = 0
bypass = 0

COMPLIMIT(outskip, inskip, dur, ingain, outgain, attack, release, threshold,
          ratio, windowlen, type, bypass, inchan=0, pctleft=.5)

