load("PAN")
rtsetparams(44100, 2)

rtinput("/snd/Public_Sounds/jimi3dSound.aiff")

setline(0,0, 2,1, 8,1, 10,0)
makegen(2, 24, 1000, 0,1, .5,.1, 1,0)

PAN(start=0, inskip=0, dur=DUR(), amp=1, inchan=0)

