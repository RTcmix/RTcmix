rtsetparams(44100, 2)
load("MULTICOMB")

rtinput("../../snd/loocher.aiff")

amp = maketable("line", 1000, 0,0, 0.5,1, 4.0,1, 4.3,0) * 0.1

// changing this random seed changes the chord
srand(1)

MULTICOMB(0, 0, DUR(), amp, cpspch(6.02), cpspch(9.05), rvbtime=1)

