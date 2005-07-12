rtsetparams(44100, 2)

rtinput("../../snd/loocher.aiff")

amp = maketable("line", 1000, 0,0, 1,1)
MIX(0, 0, DUR(), amp, 0, 0)

amp = maketable("line", 1000, 0,1, 1,0)
MIX(0.1, 0, DUR(), amp, 1, 1)

