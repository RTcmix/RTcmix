rtsetparams(44100, 2)
load("FREEVERB")

rtinput("/tmp/clave.aif")

roomsize = 0.2
damp = 70
dry = .5
wet = .2
width = 100
ringdur = 1

setline(0,1, 9,1, 10,0)

FREEVERB(0, 0, DUR(), amp=.5, roomsize, damp, dry, wet, width, ringdur)

