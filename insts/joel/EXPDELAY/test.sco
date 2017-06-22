rtsetparams(44100, 2)
load("./libMYINST.so")

rtinput("../../../snd/input.wav")

amp = maketable("line", 1000, 0,0, 1,1, 5,1, 6,0)
pan = maketable("line", 1000, 0,0, 1,1)

MYINST(st=0, inskip=0, DUR(), amp, inchan=0, pan)

