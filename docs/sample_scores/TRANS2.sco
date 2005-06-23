rtsetparams(44100, 2)
load("TRANS")

rtinput("../../snd/huhh.wav")
dur = DUR()

// Express dynamic transposition in terms of linear octaves, then convert
// to octave.pc before passing to TRANS

high = octpch(0.05)
low = octpch(-0.05)
transp = maketable("line", "nonorm", 1000, 0,0, 1,low, 3,high)
transp = makeconverter(transp, "pchoct")

// shows oct.pc output
//transp = makemonitor(transp, "display", "transp", "pch")

amp = maketable("line", 1000, 0,0, .1,1, dur-.1,1, dur,0)
pan = maketable("line", 1000, 0,0, 1,1)   // pan right to left

TRANS(0, 0, dur, amp, transp, inchan=0, pan)

