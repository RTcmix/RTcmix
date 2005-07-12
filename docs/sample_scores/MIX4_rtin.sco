set_option("record = on")  // must do this before rtsetparams
rtsetparams(44100, 2)
rtinput("AUDIO")

dur = 5

amp = maketable("line", 1000, 0,0, 1,1)
MIX(0, 0, dur, amp, 0, 0)

amp = maketable("line", 1000, 0,1, 1,0)
MIX(0.1, 0, dur, amp, 1, 1)
