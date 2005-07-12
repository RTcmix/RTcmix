rtsetparams(44100, 2)
load("JFIR")

rtinput("../../snd/input.wav")
inchan = 0
inskip = 0
dur = DUR()

start = 0
amp = 3.0
order = 300

nyq = 44100 / 2
freqresp = maketable("line", "nonorm", 5000,
   0,0, 100,0, 200,1, 700,1, 1000,0, 1500,0, 1600,.8, 2200,.8, 4000,0, nyq,0)
//plottable(freqresp)

env = maketable("line", 1000, 0,0, 1,1, 7,1, 9,0)
pan = 0.5
bypass = 1

JFIR(start, inskip, dur, amp * env, order, inchan, pan, bypass, freqresp)

