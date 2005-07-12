rtsetparams(44100, 2)
load("MULTICOMB")

rtinput("../../snd/input.wav")
dur = DUR()

amp = maketable("line", 1000, 0,0, 0.5,1, 8.6,1, 8.7,0, 8.8,0) * 0.15

// changing this random seed changes the chord
srand(1)

MULTICOMB(0, 0, dur, amp, cpspch(7.02), cpspch(8.05), rvbtime=4)

